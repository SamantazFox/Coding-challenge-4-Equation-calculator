#
# Utility functions used for test cases
# Must be included usin `source` or `.`
#

title()
{
	local blue="\033[36m"
	local reset="\033[0m"

	local line="********************"
	local void="                    "
	local twobs="\\\\"

	echo; echo $blue
	echo "  ${line}${line}  "
	printf "//%s%s%s\n" "$void" "$void" "$twobs"
	printf "||  Running test: %-23s ||\n" "$1"
	printf "%s%s%s//\n" "$twobs" "$void" "$void"
	echo "  ${line}${line}  "
	echo $reset
}


calc_test()
{
	local fail_txt="\033[1;31m FAIL \033[0m"
	local pass_txt="\033[1;32m PASS \033[0m"

	printf "Testing: %-25s" "$1"
	result=$(./calc "$1" 2>&1)

	error=$(echo "$result" | sed -E 's/^\[Error\]\s[A-Za-z0-9/.:-]+\s//')
	if ! [ -z "$error" ]; then result=$error; fi

	printf " / Result: %-25s" "$result"

	if [ "$result" = "$2" ]; then
		printf "${pass_txt}\n"
	else
		printf "${fail_txt}\n"
	fi
}
