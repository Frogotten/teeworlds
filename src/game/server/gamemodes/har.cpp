/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include <game/server/entities/flag.h>

#include "har.h"

CGameControllerHAR::CGameControllerHAR(CGameContext *pGameServer)
	: IGameController(pGameServer)
{
	m_pGameType = "H&R";

	for (int i=0; i<5; i++)
	{
		if (m_Flag[i])
		{
			m_Flag[i] = NULL;
		}
	}
	for (int i=0; i < 16; i++)
	{
		if (Red_Flag[i])
		{
			Red_Flag[i] = NULL;
		}
		if (Blue_Flag[i])
		{
			Blue_Flag[i] = NULL;
		}
	}
}

bool CGameControllerHAR::OnEntity(int Index, vec2 Pos)
{
	if(Index != ENTITY_SPAWN && 
		Index != ENTITY_SPAWN_BLUE &&
		Index != ENTITY_SPAWN_RED)
		return false;
	
	if(IGameController::OnEntity(Index, Pos))
		return true;
}

void CGameControllerHAR::OnCharacterSpawn(CCharacter *pChr)
{
	// give start equipment
	//pChr->IncreaseHealth(10);
	//pChr->IncreaseArmor(5);

	//pChr->GiveWeapon(WEAPON_HAMMER, -1);
	//pChr->GiveWeapon(WEAPON_GUN, 10);
	//pChr->GiveWeapon(WEAPON_SHOTGUN, 10);
	//pChr->GiveWeapon(WEAPON_GRENADE, 10);
	//pChr->GiveWeapon(WEAPON_LASER, 5);

	pChr->IncreaseHealth(10);	
	pChr->SetWeapon(WEAPON_HAMMER);
	pChr->GiveWeapon(WEAPON_HAMMER, -1);

	if (IGameController::GetCatcher() < IGameController::MinCatcher())
	{
		ChangeCatcher(-1, pChr->GetPlayer()->GetCID());
	}
}

void CGameControllerHAR::Tick()
{
	IGameController::Tick();

	if(GameServer()->m_World.m_ResetRequested || GameServer()->m_World.m_Paused)
		return;
	FlagTick();

	for(int i=0; i < MAX_CLIENTS; i++)
	{
		if(IGameController::IsCatcher(i) == 1)
		{
			if (str_comp_num(Server()->ClientName(i), "> ", 2))
			{
				char aBuf[20];
				str_format(aBuf, sizeof(aBuf), "> %s", Server()->ClientName(i));
				Server()->SetClientName(i, aBuf);
			}
		}
		else if (IGameController::IsCatcher(i) == 0)
		{
			if (!str_comp_num(Server()->ClientName(i), "> ", 2))
			{
				char aBuf[20];
				strcpy(aBuf, Server()->ClientName(i));
				Server()->SetClientName(i, &aBuf[2]);
			}
		}
	}

	while (IGameController::MinCatcher() > IGameController::GetCatcher())
	{
		int Random = rand()%MAX_CLIENTS;
		if ((IGameController::IsCatcher(Random) == 0) && (GameServer()->m_apPlayers[Random]->GetCharacter()))
		{
			ChangeCatcher(-1, Random);
		}
	}

	if (Server()->Tick() % Server()->TickSpeed() == 0)
	{
		for (int i=0; i < MAX_CLIENTS; i++)
		{
			if (GameServer()->GameMode)
			{
				if (IGameController::IsCatcher(i) == 1 && GameServer()->m_apPlayers[i]->GetCharacter())
					GameServer()->m_apPlayers[i]->m_Score++;
			}
			else
			{
				if (IGameController::IsCatcher(i) == 0 && GameServer()->m_apPlayers[i]->GetCharacter())
					GameServer()->m_apPlayers[i]->m_Score++;
			}
		}
	}
}

void CGameControllerHAR::FlagTick()
{
	for (int i=0; i<MAX_CLIENTS; i++)
	{
		if (GameServer()->m_apPlayers[i])
		{
			int Index = GameServer()->m_apPlayers[i]->GetFlagTeam();
			if (Index != -1)
			{
				if (m_Flag[Index-2])
				{
					if (Red_Flag[i])
					{
						Red_Flag[i]->m_Pos = m_Flag[Index-2]->m_Pos;
					}
				}
			}
		}
	}

	for(int fi = 0; fi < 5; fi++)
	{
		if (g_Config.m_SvGameFlag)
		{
			if(CountFlag() < IGameController::MinCatcher())
			{
				if(!m_Flag[fi])
				{
					CFlag* F = new CFlag(&GameServer()->m_World, fi+2);
					vec2 SpawnPos;
					if (IGameController::CanSpawn(0, &SpawnPos))
					{
						F->m_StandPos = SpawnPos; 
						F->m_Pos = SpawnPos;      
						m_Flag[fi] = F;		
						GameServer()->m_World.InsertEntity(F);

						for(int i=0; i < MAX_CLIENTS; i++)
						{
							if (IGameController::IsCatcher(i) == 1 && GameServer()->m_apPlayers[i]->GetCharacter())
							{
								if (GameServer()->m_apPlayers[i]->GetFlagTeam() == -1)
								{
									GameServer()->m_apPlayers[i]->SetFlagTeam(fi+2);
									if (!Red_Flag[i])
									{
										CFlag* F1 = new CFlag(&GameServer()->m_World, 0);
										F1->m_StandPos = m_Flag[fi]->m_Pos; 
										F1->m_Pos = m_Flag[fi]->m_Pos;      
										Red_Flag[i] = F1;		
										Red_Flag[i]->m_Index = fi+2;
										GameServer()->m_World.InsertEntity(F1);
									}
									else
									{
										Red_Flag[i]->m_StandPos = m_Flag[fi]->m_Pos; 
										Red_Flag[i]->m_Pos = m_Flag[fi]->m_Pos; 
										Red_Flag[i]->m_Index = fi+2;
									}

									if (!Blue_Flag[i])
									{
										CFlag* F1 = new CFlag(&GameServer()->m_World, 1);
										F1->m_StandPos = m_Flag[fi]->m_Pos; 
										F1->m_Pos = m_Flag[fi]->m_Pos;      
										Blue_Flag[i] = F1;		
										Blue_Flag[i]->m_Index = fi+2;
										GameServer()->m_World.InsertEntity(F1);
									}
									else
									{
										Blue_Flag[i]->m_StandPos = m_Flag[fi]->m_Pos; 
										Blue_Flag[i]->m_Pos = m_Flag[fi]->m_Pos; 
										Blue_Flag[i]->m_Index = fi+2;
									}
									break;
								}
							}
						}
						continue;
					}
				}
			}
			else if(CountFlag() > IGameController::MinCatcher())
			{ 
				if(m_Flag[fi])
				{
					bool Signal = false;
					for(int i=0; i < MAX_CLIENTS; i++)
					{
						if (IGameController::IsCatcher(i) == 1 && GameServer()->m_apPlayers[i]->GetCharacter())
						{
							if (GameServer()->m_apPlayers[i]->GetFlagTeam() == fi+2)
							{
								Signal = true;
							}
						}
					}

					if (!Signal)
					{
						GameServer()->m_World.RemoveEntity(m_Flag[fi]);
						m_Flag[fi]=0;
						continue;
					}
				}
			}
		}
		else
		{
			if(m_Flag[fi])
			{
				GameServer()->m_World.RemoveEntity(m_Flag[fi]);
				m_Flag[fi]=0;
			}	
			continue;
		}

		if(!m_Flag[fi])
			continue;

		if (Server()->Tick()%Server()->TickSpeed() == 0)
		{
			char aBu2f[250];
			str_format(aBu2f, sizeof(aBu2f), "FLAG [%d] TEAM [%d]", fi, m_Flag[fi]->m_Team);
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBu2f);
		}

		CFlag *F = m_Flag[fi];


		// flag hits death-tile or left the game layer, reset it
		if(GameServer()->Collision()->GetCollisionAt(F->m_Pos.x, F->m_Pos.y)&CCollision::COLFLAG_DEATH || F->GameLayerClipped(F->m_Pos))
		{
			GameServer()->CreateSoundGlobal(SOUND_CTF_RETURN);
			F->Reset();
			continue;
		}

		//
		if(F->m_pCarryingCharacter)
		{
			// update flag position
			F->m_Pos = F->m_pCarryingCharacter->m_Pos;
			if (F->m_pCarryingCharacter->GetPlayer())
			{
				int Index = F->m_pCarryingCharacter->GetPlayer()->GetCID();
				if (Blue_Flag[Index])
				{
					Blue_Flag[Index]->m_Pos = F->m_Pos;
				}
			}
		}
		else
		{
			CCharacter *apCloseCCharacters[MAX_CLIENTS];
			int Num = GameServer()->m_World.FindEntities(F->m_Pos, CFlag::ms_PhysSize, (CEntity**)apCloseCCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
			for(int i = 0; i < Num; i++)
			{
				if(!apCloseCCharacters[i]->IsAlive() || apCloseCCharacters[i]->GetPlayer()->GetTeam() == TEAM_SPECTATORS || GameServer()->Collision()->IntersectLine(F->m_Pos, apCloseCCharacters[i]->m_Pos, NULL, NULL))
					continue;

				int Index = apCloseCCharacters[i]->GetPlayer()->GetCID();
				if (IGameController::IsCatcher(Index) != 1 || IsFlagCharacter(Index) != -1)
					continue;

				if (GameServer()->m_apPlayers[Index]->GetFlagTeam() != fi+2)
					continue;

				if (Red_Flag[Index])
					Red_Flag[Index]->m_Index = -1;

				F->m_pCarryingCharacter = apCloseCCharacters[i];

				if (Blue_Flag[Index])
				{
					Blue_Flag[Index]->m_pCarryingCharacter = apCloseCCharacters[i];
					Blue_Flag[Index]->m_Index = fi+2;
				}
				
				GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, -1);

				break;
			}

			if(!F->m_pCarryingCharacter)
			{
				F->m_Vel.y += GameServer()->m_World.m_Core.m_Tuning.m_Gravity;
				GameServer()->Collision()->MoveBox(&F->m_Pos, &F->m_Vel, vec2(F->ms_PhysSize, F->ms_PhysSize), 0.5f);
			}
		}
	}
}

int CGameControllerHAR::CountFlag()
{
	int FlagCount = 0;

	for (int i=0; i < 5; i++)
	{
		if(m_Flag[i])
			FlagCount++;
	}

	return FlagCount;
}

int CGameControllerHAR::IsFlagCharacter( int Index )
{
	if(!g_Config.m_SvGameFlag)
		return -2;

	for(int i=0; i<5; i++)
	{
		if(!m_Flag[i])
			continue;

		CCharacter * p = m_Flag[i]->m_pCarryingCharacter;
		if(!p)
			continue;

		if(p->GetPlayer() && p->GetPlayer()->GetCID() == Index)
			return i;
	}
	return -1;
}

void CGameControllerHAR::ChangeCatcher(int Index_Old, int Index_New)
{
	// Проверка на разрешение передать кетчера	
	int Index = IsFlagCharacter(Index_Old);	

	if(Index_Old != -1 && Index_New != -1)
	{
		if(IsFlagCharacter(Index_Old) == -1)		
			return;

		if(IGameController::IsCatcher(Index_Old) && IGameController::IsCatcher(Index_New))
			return;	

		GameServer()->m_apPlayers[Index_Old]->SetFlagTeam(-1);
		if (Red_Flag[Index_Old])
		{
			Red_Flag[Index_Old]->m_Index = -1;
		}
		if (Blue_Flag[Index_Old])
		{
			Blue_Flag[Index_Old]->m_pCarryingCharacter = 0;
			Blue_Flag[Index_Old]->m_Index = -1;
		}
		if(Index > -1)
		{
			m_Flag[Index]->m_pCarryingCharacter = 0;
			GameServer()->CreateSoundGlobal(SOUND_CTF_DROP);	
		}
	}

	IGameController::ChangeCatcher(Index_Old, Index_New);			
	
	if(Index > -1)
	{		
		if (Index_New != -1 && GameServer()->m_apPlayers[Index_New])
		{
			GameServer()->m_apPlayers[Index_New]->SetFlagTeam(Index+2);

			if (Red_Flag[Index_New])
			{
				Red_Flag[Index_New]->m_StandPos = m_Flag[Index]->m_Pos; 
				Red_Flag[Index_New]->m_Pos = m_Flag[Index]->m_Pos; 
				Red_Flag[Index_New]->m_Index = Index+2;
			}
			if (Blue_Flag[Index_New])
			{
				Blue_Flag[Index_New]->m_StandPos = m_Flag[Index]->m_Pos; 
				Blue_Flag[Index_New]->m_Pos = m_Flag[Index]->m_Pos; 
				Blue_Flag[Index_New]->m_Index = Index+2;
			}
		}
		else	
		{
			if (m_Flag[Index])
			{
				GameServer()->m_World.RemoveEntity(m_Flag[Index]);
				m_Flag[Index]=0;
			}
			for (int i=0; i<MAX_CLIENTS; i++)
			{
				if (Blue_Flag[i])
				{
					if (Blue_Flag[i]->m_Index == Index+2)
					{
						GameServer()->m_World.RemoveEntity(Blue_Flag[i]);
						Blue_Flag[i]=0;
					}
				}
				if (Red_Flag[i])
				{
					if (Red_Flag[i]->m_Index == Index+2)
					{
						GameServer()->m_World.RemoveEntity(Red_Flag[i]);
						Red_Flag[i]=0;
					}
				}
			}
		}
	}
	return;
}

void CGameControllerHAR::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);
	
	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;

	pGameDataObj->m_TeamscoreRed = 0;
	pGameDataObj->m_TeamscoreBlue = 0;

	pGameDataObj->m_FlagDropTickRed = 0;
	pGameDataObj->m_FlagDropTickBlue = Server()->Tick() + Server()->TickSpeed()*30;

	pGameDataObj->m_FlagCarrierRed = FLAG_MISSING;

	if (Blue_Flag[SnappingClient])
	{
		if (Blue_Flag[SnappingClient]->m_pCarryingCharacter && Blue_Flag[SnappingClient]->m_pCarryingCharacter->GetPlayer())
		{
			pGameDataObj->m_FlagCarrierBlue = Blue_Flag[SnappingClient]->m_pCarryingCharacter->GetPlayer()->GetCID();
		}
		else
		{
			pGameDataObj->m_FlagCarrierBlue = FLAG_MISSING;
		}
	}
	else
	{
		pGameDataObj->m_FlagCarrierBlue = FLAG_MISSING;
	}
}