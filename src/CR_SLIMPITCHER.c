// read the  new_item_mode.с first!
		case CR_SLIMPITCHER:
			if (sd) {
				int i = skill_lv%11 - 1;
				int j = pc->search_inventory(sd,skill->dbs->db[skill_id].itemid[i]);
				int needed = skill->dbs->db[skill_id].itemid[i];
				int id = INDEX_NOT_FOUND;
				
				if(needed == ITEMID_WHITE_SLIM_POTION)
				{
					if (map->list[sd->bl.m].flag.gvg)
						id = pc->search_inventory(sd, ITEMID_WOESLIMPOT);
					else if(map->list[sd->bl.m].flag.battleground)
						id = pc->search_inventory(sd, ITEMID_BGSLIMPOT);
					
					if(id != INDEX_NOT_FOUND)
						j = id;
				}
				
				if (j == INDEX_NOT_FOUND || sd->inventory_data[j] == NULL || sd->status.inventory[j].amount < skill->dbs->db[skill_id].amount[i])
				{
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					return 1;
				}
				script->potion_flag = 1;
				script->potion_hp = 0;
				script->potion_sp = 0;
				script->run_use_script(sd, sd->inventory_data[j], 0);
				script->potion_flag = 0;
				//Apply skill bonuses
				i = pc->checkskill(sd,CR_SLIMPITCHER)*10
					+ pc->checkskill(sd,AM_POTIONPITCHER)*10
					+ pc->checkskill(sd,AM_LEARNINGPOTION)*5
					+ pc->skillheal_bonus(sd, skill_id);

				script->potion_hp = script->potion_hp * (100+i)/100;
				script->potion_sp = script->potion_sp * (100+i)/100;

				if(script->potion_hp > 0 || script->potion_sp > 0) {
					i = skill->get_splash(skill_id, skill_lv);
					map->foreachinarea(skill->area_sub,
					                   src->m,x-i,y-i,x+i,y+i,BL_CHAR,
					                   src,skill_id,skill_lv,tick,flag|BCT_PARTY|BCT_GUILD|1,
					                   skill->castend_nodamage_id);
				}
			} else {
				int i = skill_lv%11 - 1;
				struct item_data *item;
				i = skill->dbs->db[skill_id].itemid[i];
				item = itemdb->search(i);
				script->potion_flag = 1;
				script->potion_hp = 0;
				script->potion_sp = 0;
				script->run(item->script,0,src->id,0);
				script->potion_flag = 0;
				i = skill->get_max(CR_SLIMPITCHER)*10;

				script->potion_hp = script->potion_hp * (100+i)/100;
				script->potion_sp = script->potion_sp * (100+i)/100;

				if(script->potion_hp > 0 || script->potion_sp > 0) {
					i = skill->get_splash(skill_id, skill_lv);
					map->foreachinarea(skill->area_sub,
					                   src->m,x-i,y-i,x+i,y+i,BL_CHAR,
					                   src,skill_id,skill_lv,tick,flag|BCT_PARTY|BCT_GUILD|1,
					                   skill->castend_nodamage_id);
				}
			}
			break;