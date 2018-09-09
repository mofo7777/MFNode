//----------------------------------------------------------------------------------------------
// MFTWaveMixer_Mixer.cpp
// Copyright (C) 2013 Dumonteil David
//
// MFNode is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MFNode is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

void CMFTWaveMixer::MixAudio(const BYTE* bInput1, const BYTE* bInput2, BYTE* bOutput, const DWORD dwSize){

	DWORD sample, channel, cChannels;

	cChannels = NumChannels();

	int iInput1;
	int iInput2;
	int iResult;

	if(BitsPerSample() == 8){

		for(sample = 0; sample < dwSize; ++sample){

			for(channel = 0; channel < cChannels; ++channel){

				iInput1 = bInput1[sample * cChannels + channel];
				iInput2 = bInput2[sample * cChannels + channel];

				iResult = iInput1 + iInput2;

				if(iResult > 127){
					iResult = 127;
				}
				else if(iResult < -128){
					iResult = -128;
				}

				bOutput[sample * cChannels + channel] = (unsigned char)iResult;
			}
		}
	}
	else{

		for(sample = 0; sample < dwSize; ++sample){

			for(channel = 0; channel < cChannels; ++channel){

				iInput1 = ((short*)bInput1)[sample * cChannels + channel];
				iInput2 = ((short*)bInput2)[sample * cChannels + channel];

				iResult = iInput1 + iInput2;

				if(iResult > 32767){
					iResult = 32767;
				}
				else if(iResult < -32768){
					iResult = -32768;
				}

				((short*)bOutput)[sample * cChannels + channel] = (short)iResult;
			}
		}
	}
}