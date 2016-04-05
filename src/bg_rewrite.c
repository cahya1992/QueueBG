// ========== Battlegrounds eAmod rewrite by Kubix

// battleground.c
// original code was find in public.
// ======================================
// ====== Ищем #define HERCULES_CORE, ниже добавляем:
#include "../common/utils.h"

// крутим немного ниже, ищем #include "common/timer.h" и после
// добавляем :
#include "log.h"
#include "quest.h"

// ищем функцию bg_send_dot_remove
// меняем её на эту:
int bg_send_dot_remove(struct map_session_data *sd)
{
	struct battleground_data *bgd;
	int m;

	if( sd && sd->bg_id && (bgd = bg->team_search(sd->bg_id)) != NULL )
	{
		clif->bg_xy_remove(sd);
		if( bg->reveal_pos_sub && (m = map->mapindex2mapid(bgd->mapindex)) == sd->bl.m )
			map->foreachinmap(bg->reveal_pos_sub, m, BL_PC, sd, 2, 0xFFFFFF);
	}
	return true;
}

// после неё добавляем :

void bg_team_rewards(int bg_id, int nameid, int amount, int kafrapoints, int quest_id, const char *var, int add_value, int bg_arena, int bg_result)
{
	struct battleground_data *bg;
	struct map_session_data *sd;
	struct item_data *id;
	struct item it;
	int j, flag;

	if( amount < 1 || (bg = bg_team_search(bg_id)) == NULL || (id = itemdb->exists(nameid)) == NULL )
		return;

	bg_result = cap_value(bg_result,0,2);
	memset(&it,0,sizeof(it));
	if( nameid == 7828 || nameid == 7829 || nameid == 7773 )
	{
		it.nameid = nameid;
		it.identify = 1;
	}
	else nameid = 0;

	for( j = 0; j < MAX_BG_MEMBERS; j++ )
	{
		if( (sd = bg->members[j].sd) == NULL )
			continue;

		if( quest_id ) quest->add(sd,quest_id);
		pc_setglobalreg(sd,script->add_str(var), add_value);

		if( kafrapoints > 0 )
			pc->getcash(sd,0,kafrapoints);

		if( nameid && amount > 0 )
		{
			if( (flag = pc->additem(sd,&it,amount,LOG_TYPE_NONE)) )
				clif->additem(sd,0,0,flag);
		}
	}
}

// Ищем функцию bg_member_respawn
// == Меняем её на эту:

bool bg_member_respawn(struct map_session_data *sd) {
	struct battleground_data *bgd;
	if( sd == NULL || !pc_isdead(sd) || !sd->bg_id || (bgd = bg->team_search(sd->bg_id)) == NULL )
		return false;
	if( bgd->mapindex == 0 )
		return false; // Respawn not handled by Core
	
	if( map->list[sd->bl.m].flag.battleground )
	{
		pc->setpos(sd, bgd->mapindex, bgd->x, bgd->y, CLR_OUTSIGHT);
		status->revive(&sd->bl, 1, 100);
		return true; // Warped		
	}
	
	return false;
}


// Ищем функцию bg_create
// меняем её на эту:

int bg_create(unsigned short map_index, short rx, short ry, const char *ev, const char *dev) {
	struct battleground_data *bgd;
	bg->team_counter++;

	CREATE(bgd, struct battleground_data, 1);
	bgd->bg_id = bg->team_counter;
	bgd->count = 0;
	bgd->mapindex = map_index;
	bgd->x = rx;
	bgd->y = ry;
	safestrncpy(bgd->logout_event, ev, sizeof(bgd->logout_event));
	safestrncpy(bgd->die_event, dev, sizeof(bgd->die_event));

	memset(&bgd->members, 0, sizeof(bgd->members));
	idb_put(bg->team_db, bg->team_counter, bgd);

	return bgd->bg_id;
}

// ниже добавляем:

int bg_reveal_pos_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *pl_sd, *sd = NULL;
	int flag, color;

	pl_sd = (struct map_session_data *)bl;
	sd = va_arg(ap,struct map_session_data *); // Source
	flag = va_arg(ap,int);
	color = va_arg(ap,int);

	if( pl_sd->bg_id == sd->bg_id )
		return false; // Same Team

	clif->viewpoint(pl_sd,sd->bl.id,flag,sd->bl.x,sd->bl.y,sd->bl.id,color);
	return true;
}

void bg_team_getitem(int bg_id, int nameid, int amount)
{
	struct battleground_data *bg;
	struct map_session_data *sd;
	struct item_data *id;
	struct item it;
	int j, flag;

	if( amount < 1 || (bg = bg_team_search(bg_id)) == NULL || (id = itemdb->exists(nameid)) == NULL )
		return;
	if( nameid != 7828 && nameid != 7829 && nameid != 7773 )
		return;

	memset(&it, 0, sizeof(it));
	it.nameid = nameid;
	it.identify = 1;

	for( j = 0; j < MAX_BG_MEMBERS; j++ )
	{
		if( (sd = bg->members[j].sd) == NULL )
			continue;

		if( (flag = pc->additem(sd,&it,amount,LOG_TYPE_NONE)) )
			clif->additem(sd,0,0,flag);
	}
}

void bg_team_get_kafrapoints(int bg_id, int amount)
{
	struct battleground_data *bg;
	struct map_session_data *sd;
	int i;

	if( (bg = bg_team_search(bg_id)) == NULL )
		return;

	for( i = 0; i < MAX_BG_MEMBERS; i++ )
	{
		if( (sd = bg->members[i].sd) == NULL )
			continue;

		pc->getcash(sd, 0, amount);
	}
}


// Ищем метод battleground_defaults (внизу)
// и после bg->team_search = bg_team_search;
// добавляем:
	bg->team_rewards = bg_team_rewards;
	bg->team_getitem = bg_team_getitem;
	bg->team_get_kafrapoints = bg_team_get_kafrapoints;
	bg->reveal_pos_sub = bg_reveal_pos_sub;
	
	
	
// ======================
// battleground.h
// ======================

// ищем struct battleground_data
// после // Logout Event
// добавляем :
int reveal_pos_sub, reveal_flag;

// ищем struct battleground_interface
// после :
int (*team_db_final) (DBKey key, DBData *data, va_list ap);
// добавляем:
	void (*team_rewards) (int bg_id, int nameid, int amount, int kafrapoints, int quest_id, const char *var, int add_value, int bg_arena, int bg_result);
	void (*team_getitem) (int bg_id, int nameid, int amount);
	void (*team_get_kafrapoints) (int bg_id, int amount);
	int (*reveal_pos_sub) (struct block_list *bl, va_list ap);
	
// === Меняем void (*send_dot_remove) (struct map_session_data *sd);
// = на:
int (*send_dot_remove) (struct map_session_data *sd);
	
// ==================
// script.c
// ==================

// Ищем:
//-----------------------------------------------------------------------------
// buildin functions
//

// ===== после добавляем :

int viewpointmap_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd;
	int npc_id, type, x, y, id, color;
	npc_id = va_arg(ap,int);
	type = va_arg(ap,int);
	x = va_arg(ap,int);
	y = va_arg(ap,int);
	id = va_arg(ap,int);
	color = va_arg(ap,int);
	sd = (struct map_session_data *)bl;
	clif->viewpoint(sd,npc_id,type,x,y,id,color);
	return true;
}

BUILDIN(viewpointmap)
{
	int type,x,y,id,color,m;
	const char *map_name;

	map_name = script_getstr(st,2);
	if( (m = map->mapname2mapid(map_name)) < 0 )
		return false; // Invalid Map

	type=script_getnum(st,3);
	x=script_getnum(st,4);
	y=script_getnum(st,5);
	id=script_getnum(st,6);
	color=script_getnum(st,7);

	map->foreachinmap(viewpointmap_sub,m,BL_PC,st->oid,type,x,y,id,color);
	return true;
}

BUILDIN(flooritem)
{
	struct map_session_data *sd = script_rid2sd(st);
	struct item_data *item_data;
	int nameid, amount;

	if( sd == NULL ) return 0;

	nameid = script_getnum(st,2);
	if( (item_data = itemdb->search(nameid)) == NULL )
		return 0;

	amount = script_getnum(st,3);
	if( amount <= 0 )
		return 0;

	map->addflooritem_area(&sd->bl, 0, 0, 0, nameid, amount);
	return 0;
}

BUILDIN(flooritem2xy)
{
	struct item_data *item_data;
	int nameid, amount, m, x, y;
	const char *mapname;

	mapname = script_getstr(st,2);
	if( (m = map->mapname2mapid(mapname)) < 0 )
		return 0;

	x = script_getnum(st,3);
	y = script_getnum(st,4);
	nameid = script_getnum(st,5);
	if( (item_data = itemdb->search(nameid)) == NULL )
		return 0;

	amount = script_getnum(st,6);
	if( amount < 1 )
		return 0;

	map->addflooritem_area(NULL, m, x, y, nameid, amount);
	return 0;
}

BUILDIN(bg_getitem)
{
	int bg_id, nameid, amount;

	bg_id = script_getnum(st,2);
	nameid = script_getnum(st,3);
	amount = script_getnum(st,4);

	bg->team_getitem(bg_id, nameid, amount);
	return 0;
}

BUILDIN(bg_getkafrapoints)
{
	int bg_id, amount;

	bg_id = script_getnum(st,2);
	amount = script_getnum(st,3);

	bg->team_get_kafrapoints(bg_id, amount);
	return 0;
}

BUILDIN(bg_single) {
	const char* map_name;
	struct map_session_data *sd;
	int x, y, map_index, bg_id;

	sd = script->rid2sd(st);
	if( sd == NULL )
		return true;
	
	bg_id = script_getnum(st,2);
	map_name = script_getstr(st,3);
	if( (map_index = script->mapindexname2id(st,map_name)) == 0 )
		return true;

	x = script_getnum(st,4);
	y = script_getnum(st,5);
	
	if( bg->team_join(bg_id, sd) )
	{
		pc->setpos(sd, map_index, x, y, CLR_TELEPORT);
		script_pushint(st,1);
	}
	else
		script_pushint(st,0);

	return true;
}

BUILDIN(bg_team_reveal)
{
	struct battleground_data *bgd;
	int bg_id;

	bg_id = script_getnum(st,2);
	if( (bgd = bg->team_search(bg_id)) == NULL )
		return false;

	bgd->reveal_pos_sub = 1; // Reveal Position Mode
	return true;
}


// Меняем скрипт.команду bg_create_team
// на :
/* New Battlegrounds Stuff */
/* bg_team_create(map_name,respawn_x,respawn_y) */
/* returns created team id or -1 when fails */
BUILDIN(bg_create_team) {
	const char *map_name, *ev = "", *dev = "";
	int x, y, map_index = 0, bg_id;

	map_name = script_getstr(st,2);
	if( strcmp(map_name,"-") != 0 ) {
		map_index = script->mapindexname2id(st,map_name);
		if( map_index == 0 ) { // Invalid Map
			script_pushint(st,0);
			return true;
		}
	}

	x = script_getnum(st,3);
	y = script_getnum(st,4);
	ev = script_getstr(st,5); // Logout Event
	dev = script_getstr(st,6); // Die Event
	
	if( (bg_id = bg->create(map_index, x, y, ev, dev)) == 0 ) { // Creation failed
		script_pushint(st,-1);
	} else
		script_pushint(st,bg_id);

	return true;

}

BUILDIN(bg_reward)
{
	int bg_id, nameid, amount, kafrapoints, quest_id, add_value, bg_arena, bg_result;
	const char *var;

	bg_id = script_getnum(st,2);
	nameid = script_getnum(st,3);
	amount = script_getnum(st,4);
	kafrapoints = script_getnum(st,5);
	quest_id = script_getnum(st,6);
	var = script_getstr(st,7);
	add_value = script_getnum(st,8);
	bg_arena = script_getnum(st,9);
	bg_result = script_getnum(st,10);

	bg->team_rewards(bg_id, nameid, amount, kafrapoints, quest_id, var, add_value, bg_arena, bg_result);
	return true;
}

// ==== крутим вниз, после :
struct script_function BUILDIN[] = {
// ==========================================
// добавляем :
// ==========================================
		BUILDIN_DEF(bg_reward,"iiiiisiii"),
		BUILDIN_DEF(bg_team_reveal,"i"),
		BUILDIN_DEF(flooritem,"ii"),
		BUILDIN_DEF(flooritem2xy,"siiii"),
		BUILDIN_DEF(bg_getitem,"iii"),
		BUILDIN_DEF(bg_getkafrapoints,"ii"),
		BUILDIN_DEF(bg_single,"isii"),
		
// === меняем BUILDIN_DEF(bg_create_team,"sii"),
// на:
BUILDIN_DEF(bg_create_team,"siiss"),
// ==========================================		
// === map.h
// = ищем int (*addflooritem), и после него добавляем:
	int (*addflooritem_area)(const struct block_list* bl, int m, int x, int y, int nameid, int amount); // [Kubix]
	