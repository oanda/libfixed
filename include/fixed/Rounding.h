#ifndef FIXED_ROUNDING_H
#define FIXED_ROUNDING_H

#include <cstdint>
#include <functional>
#include <vector>

namespace fixed {

class Rounding {
  public:
    enum class Mode : uint8_t {
        //
        // Floor function, rounds down towards -infinity
        //
        //  22.77 becomes  22
        //  22.50 becomes  22
        //  22.11 becomes  22
        //  22.00 becomes  22
        //      0 becomes   0
        // -22.00 becomes -22
        // -22.11 becomes -23
        // -22.50 becomes -23
        // -22.77 becomes -23
        //
        DOWN = 0,

        //
        //  22.77 becomes  23
        //  22.50 becomes  23
        //  22.11 becomes  23
        //  22.00 becomes  22
        //      0 becomes   0
        // -22.00 becomes -22
        // -22.11 becomes -22
        // -22.50 becomes -22
        // -22.77 becomes -22
        //
        // Ceiling function, rounds up towards +infinity
        //
        UP,

        //
        // Rounds towards 0.
        //
        //  22.77 becomes  22
        //  22.50 becomes  22
        //  22.11 becomes  22
        //  22.00 becomes  22
        //      0 becomes   0
        // -22.00 becomes -22
        // -22.11 becomes -22
        // -22.50 becomes -22
        // -22.77 becomes -22
        //
        TOWARDS_ZERO,

        //
        // Rounds away from 0.
        //
        //  22.77 becomes  23
        //  22.50 becomes  23
        //  22.11 becomes  23
        //  22.00 becomes  22
        //      0 becomes   0
        // -22.00 becomes -22
        // -22.11 becomes -23
        // -22.50 becomes -23
        // -22.77 becomes -23
        //
        AWAY_FROM_ZERO,

        //
        // Rounds to the nearest full value, but for a tiebreaker if the
        // fraction of y is exactly 0.5, then q = y + 0.5
        //
        //  22.77 becomes  23
        //  22.50 becomes  23
        //  22.11 becomes  22
        //  22.00 becomes  22
        //      0 becomes   0
        // -22.00 becomes -22
        // -22.11 becomes -22
        // -22.50 becomes -22
        // -22.77 becomes -23
        //
        TO_NEAREST_HALF_UP,

        //
        // Rounds to the nearest full value, but for a tiebreaker if the
        // fraction of y is exactly 0.5, then q = y - 0.5
        //
        //  22.77 becomes  23
        //  22.50 becomes  22
        //  22.11 becomes  22
        //  22.00 becomes  22
        //      0 becomes   0
        // -22.00 becomes -22
        // -22.11 becomes -22
        // -22.50 becomes -23
        // -22.77 becomes -23
        //
        TO_NEAREST_HALF_DOWN,

        //
        // Rounds to the nearest full value, but for a tiebreaker if the
        // fraction of y is exactly 0.5, then q = y + 0.5 if y is positive, and
        // q = y − 0.5 if y is negative.
        //
        //  22.77 becomes  23
        //  22.50 becomes  23
        //  22.11 becomes  22
        //  22.00 becomes  22
        //      0 becomes   0
        // -22.00 becomes -22
        // -22.11 becomes -22
        // -22.50 becomes -23
        // -22.77 becomes -23
        //
        TO_NEAREST_HALF_AWAY_FROM_ZERO,

        //
        // Rounds to the nearest full value, but for a tiebreaker if the
        // fraction of y is exactly 0.5, then q = y - 0.5 if y is positive, and
        // q = y + 0.5 if y is negative.
        //
        //  22.77 becomes  23
        //  22.50 becomes  22
        //  22.11 becomes  22
        //  22.00 becomes  22
        //      0 becomes   0
        // -22.00 becomes -22
        // -22.11 becomes -22
        // -22.50 becomes -22
        // -22.77 becomes -23
        //
        TO_NEAREST_HALF_TOWARDS_ZERO,

        //
        // This is the default IEEE 754 rounding mode.  It is also called
        // banker's rounding, among other things.  It round to the nearest full
        // value, but for a tiebreaker it rounds to the nearest full even
        // value.
        //
        // E.g. +21.5 becomes +22, as does +22.5
        //      -21.5 becomes -22, as does -22.5
        //
        //  23.50 becomes  24
        //  23.49 becomes  23
        //  22.77 becomes  23
        //  22.50 becomes  22
        //  22.11 becomes  22
        //  22.00 becomes  22
        //      0 becomes   0
        // -22.00 becomes -22
        // -22.11 becomes -22
        // -22.50 becomes -22
        // -22.77 becomes -23
        // -23.49 becomes -23
        // -23.50 becomes -24
        //
        TO_NEAREST_HALF_TO_EVEN,

        //
        // Rounds to the nearest full value, but for a tiebreaker it rounds to
        // the nearest full odd value.
        //
        // E.g. +21.5 becomes +21, as does +20.5
        //      -21.5 becomes -21, as does -20.5
        //
        //  23.50 becomes  23
        //  23.49 becomes  23
        //  22.77 becomes  23
        //  22.50 becomes  23
        //  22.11 becomes  22
        //  22.00 becomes  22
        //      0 becomes   0
        // -22.00 becomes -22
        // -22.11 becomes -22
        // -22.50 becomes -23
        // -22.77 becomes -23
        // -23.49 becomes -23
        // -23.50 becomes -23
        //
        TO_NEAREST_HALF_TO_ODD,

        MODE_MAX_VAL // DO NOT PUT ANY MORE ENUMS AFTER THIS
    };

    //
    // Note, integerVal passed in is signed and can be negative, however to
    // cover the case where it's 0, need to pass in the negativeFlag.
    //
    template <typename T>
    static T round (
        const Mode& roundingMode,
        const T& integerVal,
        const T& decimalVal,
        const T& halfRangeVal,
        const bool negativeFlag
    );

    static const std::string& modeToString (Mode mode);

  private:
    static const std::vector<
        std::function<
            int64_t (
                const int64_t& integerVal,
                const int64_t& decimalVal,
                const int64_t& halfRangeVal,
                const bool negativeFlag
            )
        >
    > roundingAdjustments64_;

    static const std::vector<
        std::function<
            __int128_t (
                const __int128_t& integerVal,
                const __int128_t& decimalVal,
                const __int128_t& halfRangeVal,
                const bool negativeFlag
            )
        >
    > roundingAdjustments128_;

    static const std::vector<std::string> modeStrings_;

    struct RunTimeModeStringsCheck {
        RunTimeModeStringsCheck ();
    };

    //
    // This will ensure if someone updates the Mode Enum then the
    // polictyStrings_ will be updated as well.
    //
    static const RunTimeModeStringsCheck runTimeModeStringsCheck_;
};

template <>
inline int64_t Rounding::round (
    const Mode& roundingMode,
    const int64_t& integerVal,
    const int64_t& decimalVal,
    const int64_t& halfRangeVal,
    const bool negativeFlag
)
{
    auto idx = static_cast<std::underlying_type<Mode>::type> (roundingMode);

    return (
        integerVal +
        roundingAdjustments64_[idx] (
            integerVal, decimalVal, halfRangeVal, negativeFlag
        )
    );
}

template <>
inline __int128_t Rounding::round (
    const Mode& roundingMode,
    const __int128_t& integerVal,
    const __int128_t& decimalVal,
    const __int128_t& halfRangeVal,
    const bool negativeFlag
)
{
    auto idx = static_cast<std::underlying_type<Mode>::type> (roundingMode);

    return (
        integerVal +
        roundingAdjustments128_[idx] (
            integerVal, decimalVal, halfRangeVal, negativeFlag
        )
    );
}

template <typename T>
inline T Rounding::round (
    const Mode& roundingMode,
    const T& integerVal,
    const T& decimalVal,
    const T& halfRangeVal,
    const bool negativeFlag
)
{
    return round<int64_t> (
        roundingMode, integerVal, decimalVal, halfRangeVal, negativeFlag
    );
}

inline const std::string& Rounding::modeToString (Mode mode)
{
    auto idx =
        static_cast<std::underlying_type<Mode>::type> (mode);

    return modeStrings_[idx];
}

} // namespace fixed

#endif
