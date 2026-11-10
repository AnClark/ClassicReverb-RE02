#pragma once

#include <cmath>
#include <cstdint>

// ─────────────────────────────────────────────────────────────────────────────
// Silence gate: saves CPU while the input has been silent for a while.
//
// Arming: after 0.5 s of consecutive samples where BOTH the dry input and the
// PRE-wet reverb output are negligible, the gate arms (idle_ = true) and the
// caller clears the tank buffers (see the clearBufferCallback in DSP.cpp:
// resetBuffer() plus every IIR state).
//
// While armed, shouldSkipProcessing() keeps returning true — idle_ is a held
// state, not a one-shot — so processReplacing() early-outs sample after sample
// until the input resumes.  A single non-negligible input sample reopens the
// gate immediately; since the tank was cleared there is no stale tail to click.
//
// Thresholds: input 2e-6 (-114 dBFS), tank 2e-4 (-74 dBFS).  The tank bar is
// RE-02-specific and much higher than RE-03/RE-04's 2e-6 — see kGateOutEps
// below for the measured dither-floor rationale.  The PRE-wet tank criterion
// keeps a still-audible reverb tail from being cut (the original design
// comment called for this, but it was never implemented): measuring before
// Mix/Level also stops MIX=0 / Level=0 from masking a tank that is still
// ringing.
// ─────────────────────────────────────────────────────────────────────────────

class SilenceGate
{
public:
    // Called once per sample at the top of run().  Returns true while the
    // gate is held open by silence: the caller should write zeros and skip
    // all DSP for this sample.
    //
    // Design note: the zeroing deliberately stays in the caller rather than
    // being hidden behind a callback like updateInputSignal's clear-buffer
    // hook.  This predicate runs on the per-sample hot path (every sample
    // while idle), so a type-erased callback would add construction and
    // indirect-call overhead to the very path that exists to save CPU —
    // and the caller still needs the `continue` to skip the DSP anyway.
    bool shouldSkipProcessing(float inL, float inR)
    {
        if (!idle_)
            return false;

        if (peak(inL, inR) < kGateInEps)
            return true;   // stay armed — keep skipping

        // Input resumed: reopen the gate.
        idle_ = false;
        quiet_ = 0;
        return false;
    }

    // Called once per *processed* sample at the end of run() with the dry
    // input and the PRE-wet reverb output (before Mix/Level).
    // When exactly on the sample where the gate arms, it invokes clearBufferCallback
    // to clear the buffer tank once.
    //
    // ── Template functor (learning reference) ───────────────────────────
    // A "functor" is any object with operator() — a lambda is merely the
    // compiler-generated anonymous functor (captures become members).
    // Three ways to pass such a callable into a function:
    //   1. Function pointer   — zero overhead, but stateless only: a
    //                           capturing lambda cannot convert to one.
    //   2. std::function<...> — type erasure: accepts anything and can be
    //                           stored, but costs a wrapper construction
    //                           plus an uninlinable indirect call per
    //                           invocation (clone/destroy may heap-allocate).
    //   3. Template parameter — compile-time polymorphism: every callable
    //                           type instantiates its own function body,
    //                           so cb() is a direct, fully inlinable call
    //                           = zero overhead.
    // Option 3 is used here because this runs on the per-sample hot path
    // (48k calls/s): under clang -O2 the template path compiles to a plain
    // tail call, while the std::function path shows an indirect call plus
    // operator new/delete plumbing for the wrapper.
    //
    // ClearBufferFn&& is a forwarding reference: it binds both temporaries
    // (the usual lambda) and named functors alike, without copying them.
    //
    // Trade-offs to remember:
    //   - The body must stay in this header (templates need the full
    //     definition at the point of instantiation).
    //   - One copy of the body per distinct callable type (code bloat).
    //   - Heterogeneous callbacks cannot be stored for deferred/later use
    //     — storing callbacks is what std::function is for (event
    //     listeners, class members, callback registries).
    // Rule of thumb: immediate hot-path callbacks -> template functor;
    //                stored / delayed / heterogeneous -> std::function.
    // C++20 can constrain the parameter for clearer errors, e.g.
    //   template<class ClearBufferFn> requires std::invocable<ClearBufferFn&>
    //   void updateInputSignal(..., ClearBufferFn&& clearBufferCallback)
    //
    // ── Benefits using template functor in Classic Reverb ───────────────
    // ClearFn is a compile-time template parameter rather than std::function:
    // this runs for every processed sample, so type erasure would cost a
    // per-sample functor construction plus an uninlinable indirect call.
    // A template functor inlines to zero overhead and still allows the
    // caller to keep the buffer-clearing logic in plugin-owned code.
    template<class ClearBufferFn>
    void updateInputSignal(float inL, float inR, float wetL, float wetR, float sampleRate, ClearBufferFn&& clearBufferCallback)
    {
        if (idle_)
            return;

        if (peak(inL, inR) >= kGateInEps || peak(wetL, wetR) >= kGateOutEps) {
            quiet_ = 0;
            return;
        }

        if (++quiet_ >= (uint32_t)(sampleRate * 0.5f)) {
            quiet_ = 0;
            idle_ = true;
            clearBufferCallback();
        }
    }

private:
    // Peak magnitude across both channels.  The previous inL+inR sum could
    // cancel to ~0 for anti-correlated channels (false arm); a sum of the
    // magnitudes or the max cannot.
    static inline float peak(float l, float r)
    {
        const float a = std::fabs(l);
        const float b = std::fabs(r);
        return a > b ? a : b;
    }

    static constexpr float kGateInEps  = 2e-6f;  // dry input

    // RE-02-specific: 2e-4 (-74 dBFS), NOT RE-03/RE-04's 2e-6.  RE-02 injects
    // ±5e-6 dither into the reverb network EVERY sample (RE-03/RE-04 only use
    // 6e-8 noise modulation), which raises the steady-state PRE-wet floor far
    // above the -120 dBFS bar.  Measured floor peaks (peak of wetL+earlyL over
    // the last 5 s of digital silence, 25 s run):
    //   default parameters (room 150 m², damp 30%, hi-damp 40%): ~1.5e-5
    //   worst case (room 640 m², damp 0%, hi-damp 0%):           ~7.4e-5
    //   worst case @ 44.1 / 48 / 96 kHz:                   6.8e-5 .. 7.4e-5
    // A 2e-6 threshold would therefore never arm at all (the same failure
    // mode the original 1e-9 input bar had in RE-04).  2e-4 sits ~2.7x above
    // the worst-case floor, leaving headroom for long-term peak statistics;
    // it still keeps a musically audible tail from being cut.  Since arming
    // additionally requires 0.5 s continuously below the bar, the tank content
    // at clear time is bounded by the tail decay over that window — worst
    // case (long RT60 hovering just under the bar) ≈ -76 dBFS, inaudible.
    static constexpr float kGateOutEps = 2e-4f;  // PRE-wet tank

    uint32_t quiet_ = 0;    // consecutive negligible samples (gate arming)
    bool     idle_  = false; // gate armed: hold output silence, tank cleared
};
