#!/bin/sh
#
# Basic parser test cases
#

csd=${0%/*}
. ${csd}/utils.sh


title "Basic Parsing"


# Integer parsing

calc_test "0"                     "0"
calc_test "-0"                    "0"

calc_test "1"                     "1"
calc_test "-1"                    "-1"

calc_test "123"                   "123"
calc_test "-123"                  "-123"

calc_test "987 456 123"           "987456123"
calc_test "- 987 456 123"         "-987456123"

calc_test "18446744073709551615"  "18446744073709551615"

echo

# Float parsing

calc_test "0.0"                     "0"
calc_test "-0.0"                    "-0"

calc_test "1.0"                     "1"
calc_test "-1.0"                    "-1"

calc_test "0.000000000000000001"    "0"                # Truncated output
calc_test "-0.000000000000000001"   "-0"               # Truncated output

calc_test "1e0"                      "1"
calc_test "1e308"                    "1e+308"

calc_test "1.797e+308"               "1.797e+308"      # Max exponent
calc_test "4.940e-324"               "4.940e−324"      # Min exponent

calc_test "1.0e+309"                 "inf"
calc_test "-1.0e+309"                "-inf"

calc_test "18446744073709551615.0"  "18446744073709551615.0"
calc_test "18446744073709551615.0"  "18446744073709551615.0"

calc_test "1.7976931348623157e+308"  "1.797693e+308"   # Truncated output
calc_test "-1.7976931348623157e+308" "-1.797693e+308"  # Truncated output

echo

# Hexadecimal parsing

calc_test "0x0"                   "0"
calc_test "-0x0"                  "0"

calc_test "0x1"                   "1"
calc_test "-0x1"                  "-1"

calc_test "0xF"                   "15"
calc_test "-0xF"                  "-15"

calc_test "0xFF"                  "255"
calc_test "-0xFF"                 "-255"

calc_test "0xFF FF"               "65535"
calc_test "-0xFF FF"              "-65535"

calc_test "0xFF FF FF FF"         "4294967295"
calc_test "-0xFF FF FF FF"        "-4294967295"

calc_test "0xFEDCBA9876543210"    "18364758544493064720"
calc_test "-0xFEDCBA9876543210"   "-18364758544493064720"

calc_test "0xFFFF FFFF FFFF FFFF"  "18446744073709551615"
calc_test "-0x7FFF FFFF FFFF FFFF" "-9223372036854775807"

echo

# Octal parsing

calc_test "0o0"                   "0"
calc_test "-0o0"                  "0"

calc_test "0o1"                   "1"
calc_test "-0o1"                  "-1"

echo

# Binary parsing

calc_test "0b0"                   "0"
calc_test "-0b0"                  "0"

calc_test "0b1"                   "1"
calc_test "-0b1"                  "-1"

