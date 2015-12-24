/*
  wall-dcpc.c -- wall dungeon cga, palace cga module;

  Copyright (C) 2015, 2016 Bruno Félix Rezende Ribeiro <oitofelix@gnu.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <error.h>
#include "prince.h"
#include "kernel/video.h"
#include "kernel/random.h"
#include "engine/room.h"
#include "wall.h"
#include "wall-dcpc.h"

/* dungeon cga */
ALLEGRO_BITMAP *dc_wall_base, *dc_wall_left;

void
load_wall_dcpc (void)
{
  /* dungeon cga */
  dc_wall_base = load_bitmap (DC_WALL_BASE);
  dc_wall_left = load_bitmap (DC_WALL_LEFT);
}

void
unload_wall_dcpc (void)
{
  /* dungeon cga */
  al_destroy_bitmap (dc_wall_base);
  al_destroy_bitmap (dc_wall_left);
}

void
draw_wall_dcpc (ALLEGRO_BITMAP *bitmap, struct pos *p,
                enum em em)
{
  draw_wall_base_dcpc (bitmap, p, em);
  draw_wall_left_dcpc (bitmap, p, em);
  draw_wall_right (bitmap, p, em, vm);
}

void
draw_wall_base_dcpc (ALLEGRO_BITMAP *bitmap, struct pos *p,
                     enum em em)
{
  ALLEGRO_BITMAP *wall_base = NULL;

  if (em == DUNGEON) wall_base = dc_wall_base;

  struct coord c;
  draw_bitmapc (wall_base, bitmap, wall_base_coord (p, &c), 0);
}

void
draw_wall_left_dcpc (ALLEGRO_BITMAP *bitmap, struct pos *p,
                     enum em em)
{
  ALLEGRO_BITMAP *wall_left = NULL;

  if (em == DUNGEON) wall_left = dc_wall_left;

  struct coord c;
  draw_bitmapc (wall_left, bitmap, wall_coord (p, &c), 0);
}