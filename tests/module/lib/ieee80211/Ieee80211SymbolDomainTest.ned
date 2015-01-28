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

package ieee80211;

simple Ieee80211SymbolDomainTest
{
    parameters:
        string testFile;
        string ieee80211OFDMDataDecoderModule;
        string ieee80211OFDMSignalDecoderModule;
        string ieee80211OFDMDataEncoderModule;
        string ieee80211OFDMSignalEncoderModule;
        string ieee80211OFDMDataModulatorModule;
        string ieee80211OFDMSignalModulatorModule;
        string ieee80211OFDMSignalDemodulatorModule;
        string ieee80211OFDMDataDemodulatorModule;
}
