void	PPU_OneBus::RunNoSkip (int NumTicks) {
	register unsigned long TL;
	register unsigned char TC;
	register unsigned char BG;

	register int SprNum;
	register unsigned char SprSL;
	register unsigned char *CurTileData;

	register int i, y;
	for (i =0; i <NumTicks; i++) {
		if (Spr0Hit) if (!--Spr0Hit) Reg2002 |=0x40;
		Clockticks++;

		if (Clockticks ==256) {
			if (SLnum <240)	ZeroMemory(TileData, sizeof(TileData));
		} else
		if (Clockticks >=279 && Clockticks <=303) {
			if (IsRendering && SLnum ==-1) {
				VRAMAddr &=~0x7BE0;
				VRAMAddr |=IntReg &0x7BE0;
			}
		} else
		if (Clockticks ==338) {
			if (SLnum ==-1) {
				if (ShortSL && IsRendering && SkipTick)
					EndSLTicks =340;
				else
					EndSLTicks =341;
			} else
				EndSLTicks =341;
		} else
		if (Clockticks ==EndSLTicks) {
			inReset =false;
			Clockticks =0;
			SLnum++;
			NES::Scanline =TRUE;
			if (SLnum <240)
				OnScreen =TRUE;
			else
			if (SLnum ==240) {
				IsRendering =OnScreen =FALSE;
			}
			if (SLnum ==SLStartNMI) {
				Reg2002 |=0x80;
				if (Reg2000 &0x80) CPU::CPU[0]->WantNMI =TRUE;
			} else
			if (SLnum ==SLEndFrame -1) {
				if (which ==NES::WhichScreenToShow) GFX::DrawScreen();
				SLnum =-1;
				ShortSL =!ShortSL;
				if (Reg2001 &0x18) IsRendering =TRUE;
			}
		}
		// VBL flag gets cleared a cycle late
		if (SLnum ==-1 && Clockticks ==1) Reg2002 =0;
		if (SLnum ==-1 && Clockticks ==325) GetGFXPtr();	// Start rendered data at "pulse" of pre-render scanline

		if (IsRendering) {
			ProcessSprites();
			if (Clockticks &1) {
				if (IOMode) {
					RenderData[(Clockticks >> 1) &3] =0;
					if (IOMode ==2) WriteHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF, IOVal =RenderData[Clockticks >>1 &3]);
				} else
				if (ReadHandler[RenderAddr >>10] ==BusRead)
					RenderData[Clockticks >>1 &3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
				else
					RenderData[Clockticks >>1 &3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);
			}
			switch (Clockticks) {
				// BEGIN BACKGROUND
			case   0:	case   8:	case  16:	case  24:	case  32:	case  40:	case  48:	case  56:
			case  64:	case  72:	case  80:	case  88:	case  96:	case 104:	case 112:	case 120:
			case 128:	case 136:	case 144:	case 152:	case 160:	case 168:	case 176:	case 184:
			case 192:	case 200:	case 208:	case 216:	case 224:	case 232:	case 240:	case 248:
			case 320:	case 328:
				if (reg4100[0x06] &2) VRAMAddr &=~0xC00; // Idiosyncratic implementation of one-screen mirroring.
				RenderAddr =0x2000 | VRAMAddr &0xFFF; // Fetch name table byte
				EVA =0;
				break;
			case   1:	case   9:	case  17:	case  25:	case  33:	case  41:	case  49:	case  57:
			case  65:	case  73:	case  81:	case  89:	case  97:	case 105:	case 113:	case 121:
			case 129:	case 137:	case 145:	case 153:	case 161:	case 169:	case 177:	case 185:
			case 193:	case 201:	case 209:	case 217:	case 225:	case 233:	case 241:	case 249:
			case 321:	case 329:
				PatAddr =RenderData[0]<<4 | VRAMAddr >>12 | Reg2000 <<8 &0x1000; // Set up CHR-ROM address for selected tile, don't fetch yet
			case   2:	case  10:	case  18:	case  26:	case  34:	case  42:	case  50:	case  58:
			case  66:	case  74:	case  82:	case  90:	case  98:	case 106:	case 114:	case 122:
			case 130:	case 138:	case 146:	case 154:	case 162:	case 170:	case 178:	case 186:
			case 194:	case 202:	case 210:	case 218:	case 226:	case 234:	case 242:	case 250:
			case 322:	case 330:
				RenderAddr =0x23C0 | VRAMAddr &0xC00 | VRAMAddr >>4 &0x38 | VRAMAddr >>2 &0x07; // Fetch attribute table byte
				break;
			case   3:	case  11:	case  19:	case  27:	case  35:	case  43:	case  51:	case  59:
			case  67:	case  75:	case  83:	case  91:	case  99:	case 107:	case 115:	case 123:
			case 131:	case 139:	case 147:	case 155:	case 163:	case 171:	case 179:	case 187:
			case 195:	case 203:	case 211:	case 219:	case 227:	case 235:	case 243:	case 251:
				CurTileData = &TileData[Clockticks +13];
				BG =RenderData[1] >>(VRAMAddr >>4 &4 | VRAMAddr &2) &3;
				if (reg2000[0x10] &BKEXTEN) {
					TL =0;
					EVA =BG <<10;
				} else
					TL =BG *0x04040404;
				((unsigned long *)CurTileData)[0] =TL;
				((unsigned long *)CurTileData)[1] =TL;
				break;
			case 323:	case 331:
				CurTileData = &TileData[Clockticks -323];
				BG =RenderData[1] >>(VRAMAddr >>4 &4 | VRAMAddr &2) &3;
				if (reg2000[0x10] &BKEXTEN) {
					TL =0;
					EVA =BG <<10;
				} else
					TL =BG *0x04040404;
				((unsigned long *)CurTileData)[0] =TL;
				((unsigned long *)CurTileData)[1] =TL;
				break;
			case   4:	case  12:	case  20:	case  28:	case  36:	case  44:	case  52:	case  60:
			case  68:	case  76:	case  84:	case  92:	case 100:	case 108:	case 116:	case 124:
			case 132:	case 140:	case 148:	case 156:	case 164:	case 172:	case 180:	case 188:
			case 196:	case 204:	case 212:	case 220:	case 228:	case 236:	case 244:	case 252:
			case 324:	case 332:
				/* Rendering address:
					FEDC BA98 7654 3210
					-------------------
					10.B TTTT TTTT PRRR
					|| | |||| |||| |+++- Tile row
					|| | |||| |||| +---- Bitplane
					|| | ++++-++++------ Tile number from NT
					|| +---------------- Pattern table left/right
					++------------------ Indicate BG read, bitplanes 0-1
				*/
				RenderAddr =PatAddr |0x8000;
				break;
			case   5:	case  13:	case  21:	case  29:	case  37:	case  45:	case  53:	case  61:
			case  69:	case  77:	case  85:	case  93:	case 101:	case 109:	case 117:	case 125:
			case 133:	case 141:	case 149:	case 157:	case 165:	case 173:	case 181:	case 189:
			case 197:	case 205:	case 213:	case 221:	case 229:	case 237:	case 245:	case 253:
				TC =ReverseCHR[RenderData[2]]; // CHR-ROM data plane 0
				CurTileData =&TileData[Clockticks +11];
				((unsigned long *)CurTileData)[0] |=CHRLoBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |=CHRLoBit[TC >>4];
				if (reg2000[0x10] &BK16EN) {
					RenderAddr |=0x4000; // Indicate bitplanes 2-3
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[2] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
					else
						RenderDataHi[2] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);
					TC =ReverseCHR[RenderDataHi[2]]; // CHR-ROM data plane 2
					((unsigned long *)CurTileData)[0] |=CHRLoBit[TC &0xF] <<5;
					((unsigned long *)CurTileData)[1] |=CHRLoBit[TC >>4]  <<5;
				}
				break;
			case 325:	case 333:
				TC =ReverseCHR[RenderData[2]]; // CHR-ROM data plane 0
				CurTileData =&TileData[Clockticks -325];
				((unsigned long *)CurTileData)[0] |=CHRLoBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |=CHRLoBit[TC >>4];
				if (reg2000[0x10] &BK16EN) {
					RenderAddr |=0x4000; // Indicate bitplanes 2-3
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[2] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
					else
						RenderDataHi[2] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);
					TC =ReverseCHR[RenderDataHi[2]]; // CHR-ROM data plane 2
					((unsigned long *)CurTileData)[0] |=CHRLoBit[TC &0xF] <<5;
					((unsigned long *)CurTileData)[1] |=CHRLoBit[TC >>4]  <<5;
				}
				break;
			case   6:	case  14:	case  22:	case  30:	case  38:	case  46:	case  54:	case  62:
			case  70:	case  78:	case  86:	case  94:	case 102:	case 110:	case 118:	case 126:
			case 134:	case 142:	case 150:	case 158:	case 166:	case 174:	case 182:	case 190:
			case 198:	case 206:	case 214:	case 222:	case 230:	case 238:	case 246:	case 254:
			case 326:	case 334:
				RenderAddr =PatAddr |8 |0x8000;
				break;
			case   7:	case  15:	case  23:	case  31:	case  39:	case  47:	case  55:	case  63:
			case  71:	case  79:	case  87:	case  95:	case 103:	case 111:	case 119:	case 127:
			case 135:	case 143:	case 151:	case 159:	case 167:	case 175:	case 183:	case 191:
			case 199:	case 207:	case 215:	case 223:	case 231:	case 239:	case 247:	case 255:
				TC = ReverseCHR[RenderData[3]]; // CHR-ROM data plane 1
				CurTileData =&TileData[Clockticks +9];
				((unsigned long *)CurTileData)[0] |=CHRHiBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >>4];
				if (reg2000[0x10] &BK16EN) {
					RenderAddr |=0x4000; // Indicate bitplanes 2-3
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
					else
						RenderDataHi[3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);
					TC =ReverseCHR[RenderDataHi[3]]; // CHR-ROM data plane 3
					((unsigned long *)CurTileData)[0] |=CHRHiBit[TC &0xF] <<5;
					((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >>4]  <<5;
				}
				IncrementH();
				if (Clockticks ==255) IncrementV();
				break;
			case 327:	case 335:
				TC =ReverseCHR[RenderData[3]]; // CHR-ROM data plane 1
				CurTileData = &TileData[Clockticks -327];
				((unsigned long *)CurTileData)[0] |=CHRHiBit[TC & 0xF];
				((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >> 4];

				if (reg2000[0x10] &BK16EN) {
					RenderAddr |=0x4000; // Indicate bitplanes 2-3
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
					else
						RenderDataHi[3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);
					TC = ReverseCHR[RenderDataHi[3]]; // CHR-ROM data plane 3
					((unsigned long *)CurTileData)[0] |=CHRHiBit[TC &0xF] <<5;
					((unsigned long *)CurTileData)[1] |=CHRHiBit[TC >>4]  <<5;
				}
				IncrementH();
				break;
				// END BACKGROUND
				// BEGIN SPRITES
			case 256:
				VRAMAddr &=~0x41F;
				VRAMAddr |=IntReg &0x41F;
				// Fall-through
			case 264:	case 272:	case 280:	case 288:	case 296:	case 304:	case 312:
				RenderAddr =0x2000 | VRAMAddr &0xFFF;
				EVA =0;
				break;
			case 257:	case 265:	case 273:	case 281:	case 289:	case 297:	case 305:	case 313:
				break;
			case 258:	case 266:	case 274:	case 282:	case 290:	case 298:	case 306:	case 314:
				RenderAddr =0x2000 | VRAMAddr &0xFFF;
				break;
			case 259:	case 267:	case 275:	case 283:	case 291:	case 299:	case 307:	case 315:
				SprNum =Clockticks >>1 &0x1C;
				TC = SprBuff[SprNum |1];
				SprSL = (unsigned char)(SLnum -SprBuff[SprNum]);
 				if (Reg2000 &0x20) // 8x16 sprites
					PatAddr =((TC &0xFE) << 4) | ((TC &0x01) << 12) | ((SprSL &7) ^ ((SprBuff[SprNum | 2] &0x80) ? 0x17 : 0x00) ^ ((SprSL &0x8) << 1));
				else	
					PatAddr =(TC << 4) | ((SprSL &7) ^ ((SprBuff[SprNum | 2] &0x80) ? 0x7 : 0x0)) | ((Reg2000 &0x08) << 9);
				if (reg2000[0x10] &SPEXTEN) EVA =SprBuff[SprNum |2] <<8 &0x1C00;
				break;
			case 260:	case 268:	case 276:	case 284:	case 292:	case 300:	case 308:	case 316:
				RenderAddr =PatAddr |0xA000;
				break;
			case 261:	case 269:	case 277:	case 285:	case 293:	case 301:	case 309:	case 317:
				SprNum =Clockticks >>1 &0x1E;
				if (SprBuff[SprNum] &0x40)
					TC =RenderData[2];
				else
					TC =ReverseCHR[RenderData[2]];
				TL =(SprBuff[SprNum] &0x3) *0x04040404;
				CurTileData =SprData[SprNum >>2];
				((unsigned long *)CurTileData)[0] =CHRLoBit[TC &0xF] |TL;
				((unsigned long *)CurTileData)[1] =CHRLoBit[TC >>4]  |TL;

				if (reg2000[0x10] &SP16EN) {
					RenderAddr |=0x4000; // Indicate bitplanes 2-3
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[2] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
					else
						RenderDataHi[2] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);

					if (SprBuff[SprNum] &0x40)
						TC = RenderDataHi[2];
					else
						TC = ReverseCHR[RenderDataHi[2]];
					if (reg2000[0x10] &PIX16EN) {
						((unsigned long *)CurTileData)[2] =CHRLoBit[TC &0xF] |TL;
						((unsigned long *)CurTileData)[3] =CHRLoBit[TC >>4]  |TL;
					} else {
						((unsigned long *)CurTileData)[0]|=CHRLoBit[TC &0xF] <<5;
						((unsigned long *)CurTileData)[1]|=CHRLoBit[TC >>4]  <<5;
					}
				}
				break;
			case 262:	case 270:	case 278:	case 286:	case 294:	case 302:	case 310:	case 318:
				RenderAddr =PatAddr |0xA008;
				break;
			case 263:	case 271:	case 279:	case 287:	case 295:	case 303:	case 311:	case 319:
				SprNum =Clockticks >>1 &0x1E;
				if (SprBuff[SprNum] &0x40)
					TC =RenderData[3];
				else	
					TC =ReverseCHR[RenderData[3]];
				CurTileData =SprData[SprNum >>2];
				((unsigned long *)CurTileData)[0] |= CHRHiBit[TC &0xF];
				((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >>4];
				CurTileData[16] =SprBuff[SprNum];
				CurTileData[17] =SprBuff[SprNum |1];

				if (reg2000[0x10] & SP16EN) {
					RenderAddr |=0x4000; // Indicate bitplanes 2-3
					if (ReadHandler[RenderAddr >>10] ==BusRead)
						RenderDataHi[3] =CHRPointer[RenderAddr >>10][RenderAddr &0x3FF |EVA];
					else
						RenderDataHi[3] =ReadHandler[RenderAddr >>10](RenderAddr >>10, RenderAddr &0x3FF |EVA);
					if (SprBuff[SprNum] & 0x40)
						TC = RenderDataHi[3];
					else	
						TC = ReverseCHR[RenderDataHi[3]];
					if (reg2000[0x10] &PIX16EN) {
						((unsigned long *)CurTileData)[2] |=CHRHiBit[TC &0xF];
						((unsigned long *)CurTileData)[3] |=CHRHiBit[TC >>4];
						if (SprBuff[SprNum] &0x40) {
							unsigned long temp1 =((unsigned long *)CurTileData)[2];
							unsigned long temp2 =((unsigned long *)CurTileData)[3];
							((unsigned long *)CurTileData)[2] =((unsigned long *)CurTileData)[0];
							((unsigned long *)CurTileData)[3] =((unsigned long *)CurTileData)[1];
							((unsigned long *)CurTileData)[0] =temp1;
							((unsigned long *)CurTileData)[1] =temp2;
						}
					} else {
						((unsigned long *)CurTileData)[0] |= CHRHiBit[TC &0xF] <<5;
						((unsigned long *)CurTileData)[1] |= CHRHiBit[TC >>4]  <<5;
					}
				}
				break;
				// END SPRITES
			case 336:	case 338:
				RenderAddr =0x2000 | VRAMAddr &0xFFF;
			case 337:	case 339:
				break;
			case 340:
				RenderAddr =PatAddr; /* Needed for MMC3 with BG at PPU $1000 */
				break;
			}
			if (~Clockticks &1) {
				PPUCycle(RenderAddr, SLnum, Clockticks, 1);
				if (IOMode ==2) WriteHandler[RenderAddr >> 10](RenderAddr >> 10, RenderAddr & 0x3FF, RenderAddr & 0xFF);
			}
		}
		if (IOMode) {
			unsigned short addr = (unsigned short)(VRAMAddr &0x3FFF);
			if (IOMode >=5 && !IsRendering)
				PPUCycle(addr, SLnum, Clockticks, IsRendering);
			else
			if (IOMode ==2) {
				if (!IsRendering) WriteHandler[addr >>10](addr >>10, addr &0x3FF, IOVal);
			}
			else if (IOMode == 1) {
				IOMode++;
				if (!IsRendering) {
					if (ReadHandler[addr >> 10] ==BusRead)
						buf2007 =CHRPointer[addr >> 10][addr &0x3FF];
					else	
						buf2007 =(unsigned char)ReadHandler[addr >>10](addr >>10, addr &0x3FF);
				}
			}
			IOMode -=2;
			if (!IOMode) {
				if (IsRendering) {
					// while rendering, perform H and V increment, but only if not already done above
					// vertical increment done at cycle 255
					if (!(Clockticks ==255)) IncrementV();
					// horizontal increments done at 7/15/23/31/.../247 (but not 255) and 327/335
					if (!((Clockticks & 7) == 7 && !(255 <= Clockticks && Clockticks <= 319))) IncrementH();
				} else
					IncrementAddr();
			}
		}
		if (!IsRendering && !IOMode) PPUCycle(VRAMAddr, SLnum, Clockticks, 0);

		register int PalIndex, DisplayedTC;
		if (Clockticks <256 && OnScreen) {
			int maxPixel = ~(reg2000[0x10] &SP16EN && reg2000[0x10] &PIX16EN? 15: 7);
			if (Reg2001 &0x08 && (Clockticks >=8 || Settings::PPUNeverClip || Reg2001 & 0x02))
				TC = TileData[Clockticks + IntX];
			else
				TC = 0;
			DisplayedTC =showBG? TC: 0;
			
			if (Reg2001 &0x10 && (Clockticks >=8 || Reg2001 &0x04)) for (y = 0; y < SprCount; y += 4) {
				register int SprPixel = Clockticks - SprData[y >>2][17];
				register unsigned char SprDat;
				if (SprPixel &maxPixel) continue;
				SprDat = SprData[y >>2][SprPixel];
				if (SprDat &0x63) {
					if (Spr0InLine && y ==0 && TC &0x63 && Clockticks <255) {
						Spr0Hit =1;
						Spr0InLine =FALSE;
					}
					if (showOBJ && !(TC &0x63 && SprData[y >> 2][16] & 0x20)) DisplayedTC = SprDat |0x10;
					break;
				}
			
			}
			PalIndex =GetPalIndex(DisplayedTC);
		} else
			PalIndex =0x0F;
		PalIndex &= GrayScale | 0xFFC0;
		if (!(reg2000[0x10] &COLCOMP)) PalIndex |= ColorEmphasis;
		*GfxData++ =PalIndex;
		if (SLnum ==-1 && EndSLTicks ==340 && Clockticks ==339) *GfxData++ =PalIndex;
		if (UpdateIsRendering && !--UpdateIsRendering) IsRendering =Reg2001 &0x18 && SLnum <240;
		if (UpdateVRAMAddr && !--UpdateVRAMAddr) VRAMAddr =IntReg;
	}
}
