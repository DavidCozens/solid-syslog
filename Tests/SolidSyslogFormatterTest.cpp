#include "CppUTest/TestHarness.h"
#include "SolidSyslogFormatter.h"

#include <cstring>

// NOLINTBEGIN(cppcoreguidelines-macro-usage) -- test helper macros for readability; CppUTest requires macros for correct failure location

#define CREATE_FORMATTER(bufferSize) formatter = SolidSyslogFormatter_Create(storage, bufferSize)

#define CHECK_FORMATTED(expected)                                     \
    STRCMP_EQUAL(expected, SolidSyslogFormatter_AsString(formatter)); \
    LONGS_EQUAL(strlen(expected), SolidSyslogFormatter_Length(formatter))

#define CHECK_LENGTH(expected) LONGS_EQUAL(expected, SolidSyslogFormatter_Length(formatter))

// NOLINTEND(cppcoreguidelines-macro-usage)

enum
{
    TEST_BUFFER_SIZE = 64
};

// clang-format off
TEST_GROUP(SolidSyslogFormatter)
{
    SolidSyslogFormatterStorage storage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(TEST_BUFFER_SIZE)];
    // cppcheck-suppress variableScope -- member of TEST_GROUP; scope managed by CppUTest macro
    struct SolidSyslogFormatter* formatter;

    void setup() override
    {
        // cppcheck-suppress unreadVariable -- formatter is used across TEST() bodies via CppUTest macro
        CREATE_FORMATTER(TEST_BUFFER_SIZE);
    }

    void formatCharacter(char value) const { SolidSyslogFormatter_Character(formatter, value); }
    void formatBoundedString(const char* source, size_t maxLength) const { SolidSyslogFormatter_BoundedString(formatter, source, maxLength); }
    void formatUint32(uint32_t value) const { SolidSyslogFormatter_Uint32(formatter, value); }
    void formatTwoDigit(uint32_t value) const { SolidSyslogFormatter_TwoDigit(formatter, value); }
    void formatFourDigit(uint32_t value) const { SolidSyslogFormatter_FourDigit(formatter, value); }
    void formatSixDigit(uint32_t value) const { SolidSyslogFormatter_SixDigit(formatter, value); }
};

// clang-format on

TEST(SolidSyslogFormatter, CharacterWritesIntoBuffer)
{
    formatCharacter('A');

    CHECK_FORMATTED("A");
}

TEST(SolidSyslogFormatter, TwoCharactersAppendInSequence)
{
    formatCharacter('A');
    formatCharacter('B');

    CHECK_FORMATTED("AB");
}

TEST(SolidSyslogFormatter, BoundedStringWritesStringIntoBuffer)
{
    formatBoundedString("hello", 10);

    CHECK_FORMATTED("hello");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesInvalidLeadByteWithReplacementChar)
{
    formatBoundedString("\x85", 1);

    CHECK_FORMATTED("\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesSmallestContinuationByte)
{
    formatBoundedString("\x80", 1);

    CHECK_FORMATTED("\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesOverlongTwoByteEncodingPerByte)
{
    /* \xC1\x81 — overlong 2-byte form of U+0041. Per RFC 3629 §10 and Unicode
     * §3.9, each invalid byte becomes its own U+FFFD substitution. */
    formatBoundedString("\xC1\x81", 2);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesOverlongLeadC0)
{
    /* \xC0 — the other overlong 2-byte lead forbidden by RFC 3629 §4. */
    formatBoundedString("\xC0", 1);

    CHECK_FORMATTED("\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesInvalidLeadsF5ToFF)
{
    /* F5-F7: would encode codepoint > U+10FFFF (outside Unicode range).
     * F8-FF: 5+ byte prefix patterns, removed by RFC 3629.
     * \xF5 and \xFF exercise both ends of the range. */
    formatBoundedString("\xF5\xFF", 2);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesInvalidLeadsInF8ToFFMid)
{
    /* Interior of the 5+ byte prefix range — drives a mask that covers F8-FF,
     * rather than enumerating each value. */
    formatBoundedString("\xF8\xFE", 2);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringPassesValidTwoByteCodepointThrough)
{
    /* \xC2\x80 is the canonical 2-byte encoding of U+0080. Valid UTF-8. */
    formatBoundedString("\xC2\x80", 2);

    CHECK_FORMATTED("\xC2\x80");
}

TEST(SolidSyslogFormatter, BoundedStringPassesSecondValidTwoByteCodepointThrough)
{
    /* \xC3\xA9 is the canonical 2-byte encoding of U+00E9 (é). Valid UTF-8. */
    formatBoundedString("\xC3\xA9", 2);

    CHECK_FORMATTED("\xC3\xA9");
}

TEST(SolidSyslogFormatter, BoundedStringPassesHighestValidTwoByteCodepointThrough)
{
    /* \xDF\xBF is the canonical 2-byte encoding of U+07FF, the top of the 2-byte range. */
    formatBoundedString("\xDF\xBF", 2);

    CHECK_FORMATTED("\xDF\xBF");
}

TEST(SolidSyslogFormatter, BoundedStringPassesMiddleValidTwoByteCodepointThrough)
{
    /* \xCC\x80 is a valid 2-byte codepoint mid-range (U+0300, combining grave accent). */
    formatBoundedString("\xCC\x80", 2);

    CHECK_FORMATTED("\xCC\x80");
}

TEST(SolidSyslogFormatter, BoundedStringPassesValidThreeByteCodepointThrough)
{
    /* \xE0\xA0\x80 is the canonical 3-byte encoding of U+0800, the smallest
     * non-overlong 3-byte codepoint. Valid UTF-8. */
    formatBoundedString("\xE0\xA0\x80", 3);

    CHECK_FORMATTED("\xE0\xA0\x80");
}

TEST(SolidSyslogFormatter, BoundedStringPassesThreeByteCodepointWithE1LeadThrough)
{
    /* \xE1\x80\x80 is the canonical 3-byte encoding of U+1000. Forces the
     * lead byte check to generalise past \xE0. */
    formatBoundedString("\xE1\x80\x80", 3);

    CHECK_FORMATTED("\xE1\x80\x80");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesOverlongThreeByteEncodingPerByte)
{
    /* \xE0\x80\x80 — overlong 3-byte encoding of U+0000. The E0 lead
     * requires a continuation in A0-BF; \x80 is below that subrange, so the
     * sequence is ill-formed and each invalid byte becomes its own U+FFFD. */
    formatBoundedString("\xE0\x80\x80", 3);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesOverlongThreeByteEncodingAtSubrangeTop)
{
    /* \xE0\x9F\x80 — also overlong: \x9F is still below the E0 subrange
     * lower bound of \xA0. Forces the exclusion to widen beyond a single
     * hardcoded continuation byte to the full 80-9F range. */
    formatBoundedString("\xE0\x9F\x80", 3);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesUtf16SurrogateEncodingPerByte)
{
    /* \xED\xA0\x80 — UTF-8 encoding of U+D800, a UTF-16 high surrogate.
     * RFC 3629 §3 forbids encoding surrogates; the ED lead requires a
     * continuation in 80-9F, not A0-BF. */
    formatBoundedString("\xED\xA0\x80", 3);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesThreeByteLeadFollowedByAscii)
{
    /* \xE1 is a 3-byte lead; the next byte \x40 ('@') is not a valid
     * continuation (continuations must be 80-BF). Per maximal-subpart rule
     * the lead alone becomes U+FFFD; the ASCII byte passes through; the
     * stray \x80 is another U+FFFD. */
    formatBoundedString("\xE1\x40\x80", 3);

    CHECK_FORMATTED("\xEF\xBF\xBD\x40\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesTopOfThreeByteLeadRangeWhenInvalid)
{
    /* \xEF is the top of the 3-byte lead range (E0-EF). Here it isn't
     * followed by a valid continuation, so the lead alone becomes U+FFFD. */
    formatBoundedString("\xEF\x40\x80", 3);

    CHECK_FORMATTED("\xEF\xBF\xBD\x40\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringPassesValidFourByteCodepointThrough)
{
    /* \xF0\x90\x80\x80 is the canonical 4-byte encoding of U+10000, the
     * smallest non-overlong 4-byte codepoint. Valid UTF-8. */
    formatBoundedString("\xF0\x90\x80\x80", 4);

    CHECK_FORMATTED("\xF0\x90\x80\x80");
}

TEST(SolidSyslogFormatter, BoundedStringPassesFourByteCodepointWithF1LeadThrough)
{
    /* \xF1\x80\x80\x80 is the canonical 4-byte encoding of U+40000. Forces
     * the 4-byte lead check to generalise past \xF0. */
    formatBoundedString("\xF1\x80\x80\x80", 4);

    CHECK_FORMATTED("\xF1\x80\x80\x80");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesOverlongFourByteEncodingPerByte)
{
    /* \xF0\x80\x80\x80 — overlong 4-byte encoding of U+0000. The F0 lead
     * requires a continuation in 90-BF; \x80 is below that subrange. */
    formatBoundedString("\xF0\x80\x80\x80", 4);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesFourByteEncodingBeyondUnicodeRange)
{
    /* \xF4\x90\x80\x80 encodes a codepoint beyond U+10FFFF (the top of the
     * Unicode range). The F4 lead requires cont1 in 80-8F; \x90 is above
     * that subrange. */
    formatBoundedString("\xF4\x90\x80\x80", 4);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesF5AsFourByteLead)
{
    /* \xF5 is not a valid 4-byte lead — any codepoint with a F5 lead would
     * exceed U+10FFFF. RFC 3629 §3 restricts 4-byte leads to F0-F4. */
    formatBoundedString("\xF5\x80\x80\x80", 4);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesF6AsFourByteLead)
{
    /* \xF6 is not a valid 4-byte lead either — same reason as F5. */
    formatBoundedString("\xF6\x80\x80\x80", 4);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesF7AsFourByteLead)
{
    /* \xF7 is the top of the would-be 4-byte lead range but is still
     * outside the valid F0-F4 range. */
    formatBoundedString("\xF7\x80\x80\x80", 4);

    CHECK_FORMATTED("\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringTruncatesAtMaxLength)
{
    formatBoundedString("hello", 3);

    CHECK_FORMATTED("hel");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesStragglingTwoByteLeadWhenSourceTruncated)
{
    /* maxLength caps source at 1 byte, but \xC2 followed by \x80 in memory
     * would be a valid 2-byte codepoint. Per Unicode §3.9, a lead that
     * can't complete a codepoint within the input bound becomes a single
     * U+FFFD; the continuation beyond the cap is never consumed. */
    formatBoundedString("\xC2\x80", 1);

    CHECK_FORMATTED("\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesStragglingThreeByteLeadWhenSourceTruncated)
{
    /* maxLength caps source at 1 byte of an otherwise-valid 3-byte
     * sequence. The same truncation rule as the 2-byte case applies. */
    formatBoundedString("\xE0\xA0\x80", 1);

    CHECK_FORMATTED("\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringReplacesStragglingFourByteLeadWhenSourceTruncated)
{
    /* maxLength caps source at 1 byte of an otherwise-valid 4-byte
     * sequence. The same truncation rule applies. */
    formatBoundedString("\xF0\x90\x80\x80", 1);

    CHECK_FORMATTED("\xEF\xBF\xBD");
}

TEST(SolidSyslogFormatter, BoundedStringAppendsAfterCharacter)
{
    formatCharacter('<');
    formatBoundedString("hello", 10);

    CHECK_FORMATTED("<hello");
}

TEST(SolidSyslogFormatter, Uint32FormatsSingleDigit)
{
    formatUint32(7);

    CHECK_FORMATTED("7");
}

TEST(SolidSyslogFormatter, Uint32FormatsZero)
{
    formatUint32(0);

    CHECK_FORMATTED("0");
}

TEST(SolidSyslogFormatter, Uint32FormatsMultipleDigits)
{
    formatUint32(134);

    CHECK_FORMATTED("134");
}

TEST(SolidSyslogFormatter, Uint32AppendsAfterCharacter)
{
    formatCharacter('<');
    formatUint32(42);

    CHECK_FORMATTED("<42");
}

TEST(SolidSyslogFormatter, LengthStartsAtZero)
{
    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, LengthAdvancesWithWrites)
{
    formatBoundedString("hello", 10);

    CHECK_LENGTH(5);
}

TEST(SolidSyslogFormatter, AsStringReturnsFormattedContent)
{
    formatBoundedString("test", 4);

    CHECK_FORMATTED("test");
}

TEST(SolidSyslogFormatter, AsStringTrimsTruncatedTwoByteLeadAtBufferTail)
{
    /* \xC2 alone is a truncated 2-byte lead. AsString must hide it with a
     * NUL so callers never observe invalid UTF-8 at the tail. Length still
     * reports the raw byte count so the gap records what was trimmed. */
    formatCharacter('\xC2');

    STRCMP_EQUAL("", SolidSyslogFormatter_AsString(formatter));
    LONGS_EQUAL(1, SolidSyslogFormatter_Length(formatter));
}

TEST(SolidSyslogFormatter, AsStringTrimsTruncatedThreeByteLeadAtBufferTail)
{
    /* \xE0 alone is a truncated 3-byte lead. */
    formatCharacter('\xE0');

    STRCMP_EQUAL("", SolidSyslogFormatter_AsString(formatter));
    LONGS_EQUAL(1, SolidSyslogFormatter_Length(formatter));
}

TEST(SolidSyslogFormatter, AsStringTrimsTruncatedFourByteLeadAtBufferTail)
{
    /* \xF0 alone is a truncated 4-byte lead. */
    formatCharacter('\xF0');

    STRCMP_EQUAL("", SolidSyslogFormatter_AsString(formatter));
    LONGS_EQUAL(1, SolidSyslogFormatter_Length(formatter));
}

TEST(SolidSyslogFormatter, AsStringTrimsThreeByteLeadWithOnlyOneContinuation)
{
    /* \xE0\xA0 is a valid 3-byte prefix waiting for its final continuation.
     * Without the third byte the codepoint is truncated and both bytes
     * must be hidden from AsString. */
    formatCharacter('\xE0');
    formatCharacter('\xA0');

    STRCMP_EQUAL("", SolidSyslogFormatter_AsString(formatter));
    LONGS_EQUAL(2, SolidSyslogFormatter_Length(formatter));
}

TEST(SolidSyslogFormatter, AsStringTrimsFourByteLeadWithOnlyOneContinuation)
{
    /* \xF0\x90 is the first two bytes of a 4-byte sequence; still truncated. */
    formatCharacter('\xF0');
    formatCharacter('\x90');

    STRCMP_EQUAL("", SolidSyslogFormatter_AsString(formatter));
    LONGS_EQUAL(2, SolidSyslogFormatter_Length(formatter));
}

TEST(SolidSyslogFormatter, AsStringTrimsFourByteLeadWithOnlyTwoContinuations)
{
    /* \xF0\x90\x80 is the first three bytes of a 4-byte sequence; still
     * one continuation short of a complete codepoint. */
    formatCharacter('\xF0');
    formatCharacter('\x90');
    formatCharacter('\x80');

    STRCMP_EQUAL("", SolidSyslogFormatter_AsString(formatter));
    LONGS_EQUAL(3, SolidSyslogFormatter_Length(formatter));
}

TEST(SolidSyslogFormatter, ZeroSizeCharacterIsNoOp)
{
    CREATE_FORMATTER(0);

    formatCharacter('A');

    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, ZeroSizeBoundedStringIsNoOp)
{
    CREATE_FORMATTER(0);

    formatBoundedString("hello", 5);

    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, ZeroSizeUint32IsNoOp)
{
    CREATE_FORMATTER(0);

    formatUint32(42);

    CHECK_LENGTH(0);
}

TEST(SolidSyslogFormatter, OneByteBufferHoldsOnlyNullTerminator)
{
    CREATE_FORMATTER(1);

    formatCharacter('X');

    CHECK_FORMATTED("");
}

TEST(SolidSyslogFormatter, TwoDigitFormatsAllDigits)
{
    formatTwoDigit(59);

    CHECK_FORMATTED("59");
}

TEST(SolidSyslogFormatter, TwoDigitFormatsZero)
{
    formatTwoDigit(0);

    CHECK_FORMATTED("00");
}

TEST(SolidSyslogFormatter, TwoDigitFormatsMax)
{
    formatTwoDigit(99);

    CHECK_FORMATTED("99");
}

TEST(SolidSyslogFormatter, TwoDigitBeyondMaxFormatsLeastSignificant)
{
    formatTwoDigit(123);

    CHECK_FORMATTED("23");
}

TEST(SolidSyslogFormatter, FourDigitFormatsAllDigits)
{
    formatFourDigit(2009);

    CHECK_FORMATTED("2009");
}

TEST(SolidSyslogFormatter, FourDigitFormatsZero)
{
    formatFourDigit(0);

    CHECK_FORMATTED("0000");
}

TEST(SolidSyslogFormatter, FourDigitFormatsMax)
{
    formatFourDigit(9999);

    CHECK_FORMATTED("9999");
}

TEST(SolidSyslogFormatter, FourDigitBeyondMaxFormatsLeastSignificant)
{
    formatFourDigit(12345);

    CHECK_FORMATTED("2345");
}

TEST(SolidSyslogFormatter, SixDigitFormatsAllDigits)
{
    formatSixDigit(123456);

    CHECK_FORMATTED("123456");
}

TEST(SolidSyslogFormatter, SixDigitFormatsZero)
{
    formatSixDigit(0);

    CHECK_FORMATTED("000000");
}

TEST(SolidSyslogFormatter, SixDigitFormatsMax)
{
    formatSixDigit(999999);

    CHECK_FORMATTED("999999");
}

TEST(SolidSyslogFormatter, SixDigitBeyondMaxFormatsLeastSignificant)
{
    formatSixDigit(1234567);

    CHECK_FORMATTED("234567");
}

TEST(SolidSyslogFormatter, BoundedStringStopsAtBufferCapacity)
{
    CREATE_FORMATTER(4);

    formatBoundedString("abcdef", 10);

    CHECK_FORMATTED("abc");
}

TEST(SolidSyslogFormatter, BoundedStringFillsExactCapacity)
{
    CREATE_FORMATTER(4);

    formatBoundedString("abc", 3);

    CHECK_FORMATTED("abc");
}

TEST(SolidSyslogFormatter, BoundedStringWritesNothingWhenFull)
{
    CREATE_FORMATTER(4);

    formatBoundedString("abc", 3);
    formatBoundedString("xyz", 3);

    CHECK_FORMATTED("abc");
}

TEST(SolidSyslogFormatter, BoundedStringReplacementHiddenFromAsStringWhenOutputTooSmall)
{
    /* Replacement is 3 bytes. A 3-byte buffer provides 2 usable bytes
     * (one reserved for NUL), so U+FFFD cannot be fully written. The
     * formatter clamps the third byte; AsString trims the truncated
     * lead so callers see no invalid UTF-8. Length still reflects the
     * raw bytes the formatter absorbed, making the gap observable. */
    CREATE_FORMATTER(3);

    formatBoundedString("\xC3", 1);

    STRCMP_EQUAL("", SolidSyslogFormatter_AsString(formatter));
    LONGS_EQUAL(2, SolidSyslogFormatter_Length(formatter));
}

TEST(SolidSyslogFormatter, BoundedStringValidCodepointHiddenFromAsStringWhenOutputTooSmall)
{
    /* After writing 'a', only 1 usable byte remains in the 3-byte
     * buffer. The 3-byte codepoint clamps after one byte; AsString
     * trims the truncated lead so the visible tail is clean. */
    CREATE_FORMATTER(3);

    formatBoundedString("a\xE0\xA0\x80", 4);

    STRCMP_EQUAL("a", SolidSyslogFormatter_AsString(formatter));
    LONGS_EQUAL(2, SolidSyslogFormatter_Length(formatter));
}

TEST(SolidSyslogFormatter, CharacterStopsWhenFull)
{
    CREATE_FORMATTER(3);

    formatCharacter('A');
    formatCharacter('B');
    formatCharacter('C');

    CHECK_FORMATTED("AB");
}

TEST(SolidSyslogFormatter, Uint32TruncatedWhenBufferTooSmall)
{
    CREATE_FORMATTER(3);

    formatUint32(12345);

    CHECK_FORMATTED("12");
}

TEST(SolidSyslogFormatter, Uint32FitsExactly)
{
    CREATE_FORMATTER(4);

    formatUint32(123);

    CHECK_FORMATTED("123");
}

TEST(SolidSyslogFormatter, EscapedStringWithEmptyInputWritesNothing)
{
    SolidSyslogFormatter_EscapedString(formatter, "", 0);

    CHECK_FORMATTED("");
}

TEST(SolidSyslogFormatter, EscapedStringPassesOrdinaryCharacterThrough)
{
    SolidSyslogFormatter_EscapedString(formatter, "a", 1);

    CHECK_FORMATTED("a");
}

TEST(SolidSyslogFormatter, EscapedStringTruncatesAtMaxRawLength)
{
    SolidSyslogFormatter_EscapedString(formatter, "hello", 3);

    CHECK_FORMATTED("hel");
}

TEST(SolidSyslogFormatter, EscapedStringEscapesDoubleQuote)
{
    SolidSyslogFormatter_EscapedString(formatter, "a\"b", 3);

    CHECK_FORMATTED("a\\\"b");
}

TEST(SolidSyslogFormatter, EscapedStringEscapesBackslash)
{
    SolidSyslogFormatter_EscapedString(formatter, "a\\b", 3);

    CHECK_FORMATTED("a\\\\b");
}

TEST(SolidSyslogFormatter, EscapedStringEscapesCloseBracket)
{
    SolidSyslogFormatter_EscapedString(formatter, "a]b", 3);

    CHECK_FORMATTED("a\\]b");
}

TEST(SolidSyslogFormatter, EscapedStringEscapesAllThreeSpecialsInOneValue)
{
    SolidSyslogFormatter_EscapedString(formatter, "\"\\]", 3);

    CHECK_FORMATTED("\\\"\\\\\\]");
}

TEST(SolidSyslogFormatter, EscapedStringMaxRawLengthBoundsInputNotOutput)
{
    SolidSyslogFormatter_EscapedString(formatter, R"(""""")", 2);

    CHECK_FORMATTED(R"(\"\")");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringWithEmptyInputWritesNothing)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter, "", 0);

    CHECK_FORMATTED("");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringPassesPrintableCharacterThrough)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter, "A", 1);

    CHECK_FORMATTED("A");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringTruncatesAtMaxLength)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter, "hello", 3);

    CHECK_FORMATTED("hel");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringSubstitutesSpace)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter, "a b", 3);

    CHECK_FORMATTED("a?b");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringSubstitutesControlCharacter)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter,
                                            "a\x01"
                                            "b",
                                            3);

    CHECK_FORMATTED("a?b");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringSubstitutesDel)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter,
                                            "a\x7F"
                                            "b",
                                            3);

    CHECK_FORMATTED("a?b");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringSubstitutesHighBitByte)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter,
                                            "a\xC3"
                                            "b",
                                            3);

    CHECK_FORMATTED("a?b");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringPassesBangAndTildeBoundariesThrough)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter, "!~", 2);

    CHECK_FORMATTED("!~");
}

TEST(SolidSyslogFormatter, PrintUsAsciiStringTruncationBoundsSubstitution)
{
    SolidSyslogFormatter_PrintUsAsciiString(formatter,
                                            "abc\x01"
                                            "def",
                                            3);

    CHECK_FORMATTED("abc");
}
