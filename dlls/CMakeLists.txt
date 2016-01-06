
cmake_minimum_required (VERSION 2.6)
project (CSDM_SERVER)

set (CSDM_SERVER_LIBRARY server)

add_definitions (-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -DCLIENT_WEAPONS -DLINUX -D_LINUX)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpermissive ")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -Wall -Wextra")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -w -O3")

set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

include_directories (
    ./
    ../engine
    ../common
    ../pm_shared
    ../game_shared
)


set (CSDM_SERVER_SOURCES
    airtank.cpp
    animating.cpp
    animation.cpp
    armoury_entity.cpp
    bmodels.cpp
    buttons.cpp
    cbase.cpp
    client.cpp
    combat.cpp
    deadplayer_entity.cpp
    defaultai.cpp
    destroy_gamerules.cpp
    doors.cpp
    effects.cpp
    explode.cpp
    flyingmonster.cpp
    func_break.cpp
    func_headq.cpp
    func_tank.cpp
    game.cpp
    gamerules.cpp
    ggrenade.cpp
    globals.cpp
    headquarters_gamerules.cpp
    healthkit.cpp
    h_ai.cpp
    h_cine.cpp
    h_cycler.cpp
    h_export.cpp
    items.cpp
    lights.cpp
    maprules.cpp
    monstermaker.cpp
    monsters.cpp
    monsterstate.cpp
    mortar.cpp
    multiplay_gamerules.cpp
    nodes.cpp
    observer.cpp
    osprey.cpp
    pathcorner.cpp
    plane.cpp
    plats.cpp
    player.cpp
    rat.cpp
    roach.cpp
    schedule.cpp
    scripted.cpp
    sound.cpp
    soundent.cpp
    spectator.cpp
    squadmonster.cpp
    subs.cpp
    talkmonster.cpp
    teamplay_gamerules.cpp
    team_main.cpp
    tentacle.cpp
    triggers.cpp
    util.cpp
    weapons.cpp
    world.cpp
    wpn_ak47.cpp
    wpn_aug.cpp
    wpn_awp.cpp
    wpn_c4.cpp
    wpn_deagle.cpp
    wpn_famas.cpp
    wpn_flashbang.cpp
    wpn_galil.cpp
    wpn_glock.cpp
    wpn_hegrenade.cpp
    wpn_knife.cpp
    wpn_m3.cpp
    wpn_m4a1.cpp
    wpn_mp5.cpp
    wpn_sg552.cpp
    wpn_smokegrenade.cpp
    wpn_usp.cpp
    xen.cpp
    ../game_shared/voice_gamemgr.cpp
    ../pm_shared/pm_debug.c
    ../pm_shared/pm_math.c
    ../pm_shared/pm_shared.c
)

#file (GLOB CSDM_SERVER_SOURCES .cpp ../game_shared/.cpp ../pm_shared/.c)
file (GLOB CSDM_SERVER_HEADERS .h ../engine/.h ../common/.h ../game_shared/.h ../pm_shared/.h)

add_library (${CSDM_SERVER_LIBRARY} SHARED ${CSDM_SERVER_HEADERS} ${CSDM_SERVER_SOURCES})
