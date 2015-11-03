/*
  physics.c -- physics module;

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
#include "kernel/random.h"
#include "physics.h"
#include "level.h"
#include "kid.h"
#include "anim.h"
#include "room.h"

struct construct
construct (struct pos p)
{
  p = norm_pos (p, true);
  return level->construct[p.room][p.floor][p.place];
}

struct construct
construct_rel (struct pos p, int floor, int place)
{
  p.floor += floor;
  p.place += place;
  return construct (p);
}

struct pos
norm_pos (struct pos p, bool floor_first)
{
  if (p.room >= ROOMS)
    error (-1, 0, "%s: room out of range (%u)", __func__, p.room);

  if (floor_first) {
    p = norm_pos_floor (p);
    p = norm_pos_place (p);
  } else {
    p = norm_pos_place (p);
    p = norm_pos_floor (p);
  }

  return p;
}

struct pos
norm_pos_floor (struct pos p)
{
  unsigned int room_offset;
  if (p.floor < 0) {
    unsigned int q = (-p.floor) / FLOORS;
    unsigned int r = (-p.floor) % FLOORS;
    p.floor = r ? FLOORS - r : 0;
    for (room_offset = r ? q + 1 : q; room_offset > 0; room_offset--)
      p.room = level->link[p.room].a;
  } else if (p.floor >= FLOORS) {
    unsigned int q = p.floor / FLOORS;
    unsigned int r = p.floor % FLOORS;
    p.floor = r;
    for (room_offset = q; room_offset > 0; room_offset--)
      p.room = level->link[p.room].b;
  }
  return p;
}

struct pos
norm_pos_place (struct pos p)
{
  unsigned int room_offset;
  if (p.place < 0) {
    unsigned int q = (-p.place) / PLACES;
    unsigned int r = (-p.place) % PLACES;
    p.place = r ? PLACES - r : 0;
    for (room_offset = r ? q + 1 : q; room_offset > 0; room_offset--)
      p.room = level->link[p.room].l;
  } else if (p.place >= PLACES) {
    unsigned int q = p.place / PLACES;
    unsigned int r = p.place % PLACES;
    p.floor = r;
    for (room_offset = q; room_offset > 0; room_offset--)
      p.room = level->link[p.room].r;
  }
  return p;
}

unsigned int
prandom_pos (struct pos p, unsigned int i, unsigned int max)
{
  return
    prandom_uniq (p.room + p.floor * PLACES + p.place + i, max);
}

struct pos
pos_xy (unsigned int room, int x, int y)
{
  struct pos p;

  unsigned int qx = x / PLACE_WIDTH;
  unsigned int rx = x % PLACE_WIDTH;
  unsigned int qy = y / PLACE_HEIGHT;
  unsigned int ry = y % PLACE_HEIGHT;

  p.room = room;
  p.place = (rx < 13) ? qx - 1 : qx;
  p.floor = (ry < 3) ? qy - 1 : qy;

  if (x < 0) p.place = -1;
  if (y < 0) p.floor = -1;

  return p;
}

struct pos
pos (struct anim a)
{
  int w = al_get_bitmap_width (a.frame);
  return pos_xy (a.room, (a.dir == LEFT) ? a.x : a.x + w - 1, a.y);
}

bool
is_colliding (struct anim a)
{
  if (! a.collision) return false;

  a.x += (a.dir == LEFT) ? 0 : 0;
  struct pos p = pos (a);
  struct construct c = construct (p);
  if (c.fg == WALL) {
    return true;
  } else return false;
}

int
dist_collision (struct anim a)
{
  int inc = (a.dir == LEFT) ? -1 : +1;
  int x = a.x;

  if (! is_colliding (a))
    while (! is_colliding (a) && abs (x - a.x) != PLACE_WIDTH)
      a.x += inc;
  else
    while (is_colliding (a) && abs (x - a.x) != PLACE_WIDTH)
      a.x -= inc;

  return inc * (a.x - x);
}

bool
is_falling (struct anim a)
{
  if (! a.fall) return false;

  a.x += (a.dir == LEFT) ? 4 : -5;
  struct pos p = pos (a);
  struct construct c = construct (p);

  if (c.fg == NO_FLOOR) {
    return true;
  }

  return false;
}

int
dist_fall (struct anim a)
{
  int inc = (a.dir == LEFT) ? -1 : +1;
  int x = a.x;

  if (! is_falling (a))
    while (! is_falling (a) && abs (x - a.x) != PLACE_WIDTH)
      a.x += inc;
  else
    while (is_falling (a) && abs (x - a.x) != PLACE_WIDTH)
      a.x -= inc;

  return inc * (a.x - x);
}

bool
is_hitting_ceiling (struct anim a)
{
  if (! a.ceiling) return false;

  struct pos top = room_pos_tl (a);
  struct pos bottom = room_pos_br (a);
  struct construct c = construct (top);

  if (! is_kid_vjump ()) return false;

  if (top.floor != bottom.floor && c.fg != NO_FLOOR) return true;

  return false;
}

void
to_collision_edge (struct anim *a)
{
  int dc = dist_collision (*a);
  int dir = (a->dir == LEFT) ? -1 : +1;
  a->x += dir * ((abs (dc) < PLACE_WIDTH) ? dc - 1 : 0);
  printf ("dc = %i\n", dc);
}

void
to_fall_edge (struct anim *a)
{
  int df = dist_fall (*a);
  int dir = (a->dir == LEFT) ? -1 : +1;
  a->x += dir * ((abs (df) < PLACE_WIDTH) ? df - 1 : 0);
  printf ("df = %i\n", df);
}

void
apply_physics (struct anim *a, ALLEGRO_BITMAP *frame,
               int dx, int dy)
{
  struct anim na = next_anim (*a, frame, dx, dy);

  if (is_hitting_ceiling (na)) {
    na.odraw = na.draw;
    na.draw = na.ceiling;
  } else if (is_colliding (na)
      && na.draw != na.collision) {
    na.odraw = na.draw;
    na.draw = na.collision;
    to_collision_edge (&na);
  } else if (is_falling (na)
             && na.draw != na.fall
             && na.draw != na.collision) {
    na.odraw = na.draw;
    na.draw = na.fall;
  }

  (*a) = na;
}
