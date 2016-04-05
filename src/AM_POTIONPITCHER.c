// Сначала прочтите файл new_item_mode.с

		case AM_POTIONPITCHER:
			{
				int i,sp = 0;
				int64 hp = 0;
				if( dstmd && dstmd->class_ == MOBID_EMPELIUM ) {
					map->freeblock_unlock();
					return 1;
				}
				if( sd ) {
					int x,bonus=100, potion = min(500+skill_lv,505);
					x = skill_lv%11 - 1;
					i = pc->search_inventory(sd,skill->dbs->db[skill_id].itemid[x]);
					int needed = skill->dbs->db[skill_id].itemid[x];
					int id = INDEX_NOT_FOUND;
					
					if(needed == ITEMID_WHITE_POTION)
					{
						if (map->list[sd->bl.m].flag.gvg)
							id = pc->search_inventory(sd, ITEMID_WOEWHITEPOT);
						else if(map->list[sd->bl.m].flag.battleground)
							id = pc->search_inventory(sd, ITEMID_BGWHITEPOT);
						
						if(id != INDEX_NOT_FOUND)
							i = id;
					}
					else if(needed == ITEMID_BLUE_POTION)
					{
						if (map->list[sd->bl.m].flag.gvg)
							id = pc->search_inventory(sd, ITEMID_WOEBLUEPOT);
						else if(map->list[sd->bl.m].flag.battleground)
							id = pc->search_inventory(sd, ITEMID_BGBLUEPOT);
						else id = pc->search_inventory(sd,skill->dbs->db[skill_id].itemid[x]);
						
						if(id != INDEX_NOT_FOUND)
							i = id;
					}
					
					/* || skill->dbs->db[skill_id].itemid[x] <= 0*/
					if (i == INDEX_NOT_FOUND) {
						clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
						map->freeblock_unlock();
						return 1;
					}
					if(sd->inventory_data[i] == NULL || sd->status.inventory[i].amount < skill->dbs->db[skill_id].amount[x]) {
						clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
						map->freeblock_unlock();
						return 1;
					}
					if( skill_id == AM_BERSERKPITCHER ) {
						if( dstsd && dstsd->status.base_level < (unsigned int)sd->inventory_data[i]->elv ) {
							clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
							map->freeblock_unlock();
							return 1;
						}
					}
					script->potion_flag = 1;
					script->potion_hp = script->potion_sp = script->potion_per_hp = script->potion_per_sp = 0;
					script->potion_target = bl->id;
					script->run_use_script(sd, sd->inventory_data[i], 0);
					script->potion_flag = script->potion_target = 0;
					if( sd->sc.data[SC_SOULLINK] && sd->sc.data[SC_SOULLINK]->val2 == SL_ALCHEMIST )
						bonus += sd->status.base_level;
					if( script->potion_per_hp > 0 || script->potion_per_sp > 0 ) {
						hp = tstatus->max_hp * script->potion_per_hp / 100;
						hp = hp * (100 + pc->checkskill(sd,AM_POTIONPITCHER)*10 + pc->checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
						if( dstsd ) {
							sp = dstsd->status.max_sp * script->potion_per_sp / 100;
							sp = sp * (100 + pc->checkskill(sd,AM_POTIONPITCHER)*10 + pc->checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
						}
					} else {
						if( script->potion_hp > 0 ) {
							hp = script->potion_hp * (100 + pc->checkskill(sd,AM_POTIONPITCHER)*10 + pc->checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
							hp = hp * (100 + (tstatus->vit<<1)) / 100;
							if( dstsd )
								hp = hp * (100 + pc->checkskill(dstsd,SM_RECOVERY)*10) / 100;
						}
						if( script->potion_sp > 0 ) {
							sp = script->potion_sp * (100 + pc->checkskill(sd,AM_POTIONPITCHER)*10 + pc->checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
							sp = sp * (100 + (tstatus->int_<<1)) / 100;
							if( dstsd )
								sp = sp * (100 + pc->checkskill(dstsd,MG_SRECOVERY)*10) / 100;
						}
					}

					for(i = 0; i < ARRAYLENGTH(sd->itemhealrate) && sd->itemhealrate[i].nameid; i++) {
						if (sd->itemhealrate[i].nameid == potion) {
							hp += hp * sd->itemhealrate[i].rate / 100;
							sp += sp * sd->itemhealrate[i].rate / 100;
							break;
						}
					}

					if( (i = pc->skillheal_bonus(sd, skill_id)) ) {
						hp += hp * i / 100;
						sp += sp * i / 100;
					}
				} else {
					//Maybe replace with potion_hp, but I'm unsure how that works [Playtester]
					switch (skill_lv) {
						case 1: hp = 45; break;
						case 2: hp = 105; break;
						case 3: hp = 175; break;
						default: hp = 325; break;
					}
					hp = (hp + rnd()%(skill_lv*20+1)) * (150 + skill_lv*10) / 100;
					hp = hp * (100 + (tstatus->vit<<1)) / 100;
					if( dstsd )
						hp = hp * (100 + pc->checkskill(dstsd,SM_RECOVERY)*10) / 100;
				}
				if (dstsd && (i = pc->skillheal2_bonus(dstsd, skill_id)) != 0) {
					hp += hp * i / 100;
					sp += sp * i / 100;
				}
				if( tsc && tsc->count ) {
					if( tsc->data[SC_CRITICALWOUND] ) {
						hp -= hp * tsc->data[SC_CRITICALWOUND]->val2 / 100;
						sp -= sp * tsc->data[SC_CRITICALWOUND]->val2 / 100;
					}
					if( tsc->data[SC_DEATHHURT] ) {
						hp -= hp * 20 / 100;
						sp -= sp * 20 / 100;
					}
					if( tsc->data[SC_WATER_INSIGNIA] && tsc->data[SC_WATER_INSIGNIA]->val1 == 2 ) {
						hp += hp / 10;
						sp += sp / 10;
					}
				}
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				if( hp > 0 || (skill_id == AM_POTIONPITCHER && sp <= 0) )
					clif->skill_nodamage(NULL,bl,AL_HEAL,(int)hp,1);
				if( sp > 0 )
					clif->skill_nodamage(NULL,bl,MG_SRECOVERY,sp,1);
		#ifdef RENEWAL
				if( tsc && tsc->data[SC_EXTREMITYFIST2] )
					sp = 0;
		#endif
				status->heal(bl,(int)hp,sp,0);
			}
			break;