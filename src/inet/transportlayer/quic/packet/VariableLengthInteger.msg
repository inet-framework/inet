//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

namespace inet::quic;

class VariableLengthInteger {
    uint64_t intValue;
}
cplusplus(VariableLengthInteger) {{
  public:
    VariableLengthInteger(uint64_t intValue) {
        setIntValue(intValue);
    }
    operator uint64_t() const { return intValue; }
    size_t getSize() const {
        if (intValue < 64ull) {
            return 1;
        }
        if (intValue < (1ull << 14)) {
            return 2;
        }
        if (intValue < (1ull << 30)) {
            return 4;
        }
        return 8;
    }
}}


