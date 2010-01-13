/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */


/* client */
MACRO_CONFIG_INT(cl_predict, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Predict client movements")
MACRO_CONFIG_INT(cl_nameplates, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show nameplates")
MACRO_CONFIG_INT(cl_nameplates_always, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Always show nameplats disregarding of distance")
MACRO_CONFIG_INT(cl_autoswitch_weapons, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Auto switch weapon on pickup")

MACRO_CONFIG_INT(cl_showfps, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show ingame FPS counter")

MACRO_CONFIG_INT(cl_airjumpindicator, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(cl_threadsoundloading, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(cl_warning_teambalance, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Warn about team balance")

MACRO_CONFIG_INT(cl_mouse_deadzone, 300, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(cl_mouse_followfactor, 60, 0, 200, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(cl_mouse_max_distance, 800, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(ed_showkeys, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(cl_flow, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(cl_show_welcome, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(cl_motd_time, 10, 0, 100, CFGFLAG_CLIENT|CFGFLAG_SAVE, "How long to show the server message of the day")

MACRO_CONFIG_STR(cl_version_server, 100, "version.teeworlds.com", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Server to use to check for new versions")

//Hud-Mod
MACRO_CONFIG_INT(cl_render_time, 1, 0, 1, CFGFLAG_CLIENT, "Turnes the Server time on or off")
MACRO_CONFIG_INT(cl_render_scoreboard, 1, 0, 1, CFGFLAG_CLIENT, "Turnes the game over scoreboard on or off")
MACRO_CONFIG_INT(cl_render_warmup, 1, 0, 1, CFGFLAG_CLIENT, "Turnes warmup timer on or off")
MACRO_CONFIG_INT(cl_render_broadcast, 1, 0, 1, CFGFLAG_CLIENT, "Turnes broadcast on or off")
MACRO_CONFIG_INT(cl_render_hp, 1, 0, 1, CFGFLAG_CLIENT, "Turnes the hp display on or off")
MACRO_CONFIG_INT(cl_render_ammo, 1, 0, 1, CFGFLAG_CLIENT, "Turnes the ammo display on or off")
MACRO_CONFIG_INT(cl_render_crosshair, 1, 0, 1, CFGFLAG_CLIENT, "Turnes the crosshair on or off")
MACRO_CONFIG_INT(cl_render_score, 1, 0, 1, CFGFLAG_CLIENT, "Turnes the small score board on or off")
MACRO_CONFIG_INT(cl_render_viewmode, 1, 0, 1, CFGFLAG_CLIENT, "Turnes viewmode display on or off")
MACRO_CONFIG_INT(cl_render_infomsg, 1, 0, 1, CFGFLAG_CLIENT, "Turnes info messages display on or off")
MACRO_CONFIG_INT(cl_render_servermsg, 1, 0, 1, CFGFLAG_CLIENT, "Turnes in game server messages on or off")
MACRO_CONFIG_INT(cl_render_chat, 1, 0, 1, CFGFLAG_CLIENT, "Turnes in game chat on or off")
MACRO_CONFIG_INT(cl_render_kill, 1, 0, 1, CFGFLAG_CLIENT, "Turnes kill messages on or off")
MACRO_CONFIG_INT(cl_render_vote, 1, 0, 1, CFGFLAG_CLIENT, "Turnes Vote messages on or off")

MACRO_CONFIG_INT(cl_clear_hud, 0, 0, 1, CFGFLAG_CLIENT, "Turnes hud on or off")
MACRO_CONFIG_INT(cl_clear_all, 0, 0, 1, CFGFLAG_CLIENT, "Cleans the whole screen")

MACRO_CONFIG_INT(cl_servermsgsound, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Turnes servermessage sound on or off")
MACRO_CONFIG_INT(cl_chatsound, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Turnes chat sound on or off")

MACRO_CONFIG_INT(player_use_custom_color, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toggles usage of custom colors")
MACRO_CONFIG_INT(player_color_body, 65408, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player body color")
MACRO_CONFIG_INT(player_color_feet, 65408, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player feet color")
MACRO_CONFIG_STR(player_skin, 64, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player skin")
MACRO_CONFIG_INT(cl_default_skin_only, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Default skin only")
MACRO_CONFIG_INT(cl_showduelscores, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show small scoreboard in 1on1 game")
MACRO_CONFIG_INT(gfx_eyecandy, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Advanced graphic effects")
#ifndef CONF_TRUNC
MACRO_CONFIG_INT(cl_reconnect_time, 10, 0, 100, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Auto reconnection if disconnected")
MACRO_CONFIG_INT(cl_spectator_zoom, 100, 10, 500, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Zoom in spectator mode")
#endif

MACRO_CONFIG_INT(ui_page, 5, 0, 9, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface page")
MACRO_CONFIG_INT(ui_toolbox_page, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toolbox page")
MACRO_CONFIG_STR(ui_server_address, 128, "localhost:8303", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface server address")
MACRO_CONFIG_INT(ui_scale, 100, 1, 100000, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface scale")

MACRO_CONFIG_INT(ui_color_hue, 160, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color hue")
MACRO_CONFIG_INT(ui_color_sat, 70, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color saturation")
MACRO_CONFIG_INT(ui_color_lht, 175, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color lightness")
MACRO_CONFIG_INT(ui_color_alpha, 228, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface alpha")

MACRO_CONFIG_INT(ui_new_background, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "New animated background")
MACRO_CONFIG_INT(ui_new_background_type, 0, 0, 3, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Type of new background (grass, jungle, desert, winter)")
MACRO_CONFIG_INT(gfx_smileys, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Smileys in chat")

MACRO_CONFIG_INT(cl_gameover_screenshot, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Make screenshot after game over")
MACRO_CONFIG_INT(cl_autorecord, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Automatical demo recording")
MACRO_CONFIG_INT(cl_autorecord_time, 10, 0, 10000, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Minimal demo length for saving automatical recorded demo")
MACRO_CONFIG_INT(cl_new_scoreboard, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "New detailed scoreboard")

MACRO_CONFIG_INT(gfx_noclip, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Disable clipping")

/* server */
MACRO_CONFIG_INT(sv_warmup, 0, 0, 0, CFGFLAG_SERVER, "Number of seconds to do warpup before round starts")
MACRO_CONFIG_STR(sv_motd, 900, "", CFGFLAG_SERVER, "Message of the day to display for the clients")
MACRO_CONFIG_INT(sv_teamdamage, 0, 0, 1, CFGFLAG_SERVER, "Team damage")
MACRO_CONFIG_STR(sv_maprotation, 768, "", CFGFLAG_SERVER, "Maps to rotate between")
MACRO_CONFIG_INT(sv_rounds_per_map, 1, 1, 100, CFGFLAG_SERVER, "Number of rounds on each map before rotating")
MACRO_CONFIG_INT(sv_powerups, 1, 0, 1, CFGFLAG_SERVER, "Allow powerups like ninja")
MACRO_CONFIG_INT(sv_scorelimit, 20, 0, 1000, CFGFLAG_SERVER, "Score limit (0 disables)")
MACRO_CONFIG_INT(sv_timelimit, 0, 0, 1000, CFGFLAG_SERVER, "Time limit in minutes (0 disables)")
MACRO_CONFIG_STR(sv_gametype, 32, "dm", CFGFLAG_SERVER, "Game type (dm, tdm, ctf)")
MACRO_CONFIG_INT(sv_tournament_mode, 0, 0, 1, CFGFLAG_SERVER, "Tournament mode. When enabled, players joins the server as spectator")
MACRO_CONFIG_INT(sv_spamprotection, 1, 0, 1, CFGFLAG_SERVER, "Spam protection")

MACRO_CONFIG_INT(sv_spectator_slots, 0, 0, MAX_CLIENTS, CFGFLAG_SERVER, "Number of slots to reserve for spectators")
MACRO_CONFIG_INT(sv_teambalance_time, 1, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before autobalancing teams")

MACRO_CONFIG_INT(sv_vote_kick, 1, 0, 1, CFGFLAG_SERVER, "Allow voting to kick players")
MACRO_CONFIG_INT(sv_vote_kick_bantime, 5, 0, 1440, CFGFLAG_SERVER, "The time to ban a player if kicked by vote. 0 makes it just use kick")
MACRO_CONFIG_INT(sv_vote_scorelimit, 0, 0, 1, CFGFLAG_SERVER, "Allow voting to change score limit")
MACRO_CONFIG_INT(sv_vote_timelimit, 0, 0, 1, CFGFLAG_SERVER, "Allow voting to change time limit")

//Chat-Mod
MACRO_CONFIG_INT(cl_change_sound, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Turns special chat sound on or off")
MACRO_CONFIG_INT(cl_change_color, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Turns special chat sound on or off")
MACRO_CONFIG_STR(cl_search_name, 64, "nameless tee", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Names to search for in chat messages")
MACRO_CONFIG_INT(cl_anti_spam, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Disables multiple messages")
MACRO_CONFIG_INT(cl_block_spammer, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Disables chat messages for player you want to ignore")
MACRO_CONFIG_STR(cl_spammer_name, 64, "nameless tee", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Names of players you want to ignore")

/* debug */
#ifdef CONF_DEBUG /* this one can crash the server if not used correctly */
	MACRO_CONFIG_INT(dbg_dummies, 0, 0, 15, CFGFLAG_SERVER, "")
#endif

MACRO_CONFIG_INT(dbg_focus, 0, 0, 1, CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(dbg_tuning, 0, 0, 1, CFGFLAG_CLIENT, "")
