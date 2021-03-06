#! /bin/sh

# Copyright (C) 2015-2016 Chase
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#Test parsing trailing comma

#Object trailing comma
JSON_STR=$(cat <<'EOF'
	{
		"n": 1,
		"t": true,
	}
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin FAIL
if $?; then
	exit $?
fi

#Array trailing comma
JSON_STR=$(cat <<'EOF'
	{
		"n": [
			1, 2, 3,
		]
	}
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin FAIL
exit $?
