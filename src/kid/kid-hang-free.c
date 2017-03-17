/*
  kid-hang-free.c -- kid hang free module;

  Copyright (C) 2015, 2016, 2017 Bruno Félix Rezende Ribeiro
  <oitofelix@gnu.org>

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

#include "mininim.h"

static bool flow (struct actor *k);
static bool physics_in (struct actor *k);
static void physics_out (struct actor *k);

void
kid_hang_free (struct actor *k)
{
  k->oaction = k->action;
  k->oi = k->i;
  k->action = kid_hang_free;
  k->f.flip = (k->f.dir == RIGHT) ?  ALLEGRO_FLIP_HORIZONTAL : 0;

  if (! flow (k)) return;
  if (! physics_in (k)) return;
  next_frame (&k->f, &k->f, &k->fo);
  physics_out (k);
}

static bool
flow (struct actor *k)
{
  struct pos np;

  if (k->oaction != kid_hang_free)
    k->i = 5, k->j = -1, k->wait = 3, k->reverse = true;

  bool hang_back = ((k->f.dir == LEFT) ? k->key.right : k->key.left)
    && ! k->key.up && k->key.shift;

  int back_dir = (k->f.dir == LEFT) ? RIGHT : LEFT;

    /* hang back */
  if (movements == NATIVE_MOVEMENTS
      && k->i >= 7
      && hang_back && is_hangable_pos (&k->hang_pos, back_dir)) {
    play_audio (&hang_on_fall_audio, NULL, k->id);
    kid_haptic (k, KID_HAPTIC_HANG);
    kid_turn (k);
    return false;
  }

  /* climb */
  if ((k->i < 5 || k->j > -1
       || k->hang_caller != kid_unclimb)
      && k->key.up
      && ! ((k->key.left || k->key.right)
            && k->hang_caller == kid_unclimb)
      && ! k->hang_limit) {
    kid_climb (k);
    return false;
  }

  /* release */
  if ((! k->key.shift || k->hang_limit
       || get_hanged_tile (&k->hang_pos, k->f.dir) == NO_FLOOR)
      && (k->i < 5 || k->j > -1)) {
    int dir = (k->f.dir == LEFT) ? -1 : +1;
    k->hang_limit = false;
    if (! is_traversable (&k->hang_pos)
        && k->i >= 4) {
      int dx = (k->f.dir == LEFT) ? +7 : PLACE_WIDTH + 9;
      int dy = -8;
      place_actor (k, &k->hang_pos, dx, dy, "KID", "VJUMP", 13);
      kid_vjump (k);
      return false;
    }
    if (! is_traversable (prel (&k->hang_pos, &np, +0, dir))
        && k->i <= 4) {
      int dx = (k->f.dir == LEFT) ? +7 : PLACE_WIDTH + 5;
      int dy = -8;
      place_actor (k, &k->hang_pos, dx, dy, "KID", "VJUMP", 13);
      kid_vjump (k);
      return false;
    }
    if (is_traversable (&k->hang_pos)
        && k->i >= 4) {
      int dx = (k->f.dir == LEFT) ? +10 : +22;
      int dy = +4;
      place_actor (k, &k->hang_pos, dx, dy, "KID", "FALL", 0);
      kid_fall (k);
      return false;
    }
    if (is_traversable (prel (&k->hang_pos, &np, +0, dir))
        && k->i <= 4) {
      int dx = (k->f.dir == LEFT) ? -10 : PLACE_WIDTH + 10;
      int dy = +12;
      place_actor (k, &k->hang_pos, dx, dy, "KID", "FALL", 0);
      kid_fall (k);
      return false;
    }
  }

  if (k->reverse && k->i > 0) {
    if (k->i == 4  && k->j++ > 0) k->hang_limit = true;
    k->i--;
  } else if (k->reverse && k->i == 0) {
    if (k->wait == 0) {
      k->reverse = false; k->i++;
    } else k->wait--;
  } else if (! k->reverse
             && ((k->j == 0 && k->i < 12)
                 || (k->j > 0 && k->i < 9))) k->i++;
  else if (! k->reverse
           && ((k->j == 0 && k->i == 12)
               || (k->j > 0 && k->i == 9))) {
    k->reverse = true; k->i--;
  }

  k->fo.b = actor_bitmap (k, "KID", "HANG", k->i);
  k->fo.dx = (k->reverse) ? -actor_bitmap_dx (k, "KID", "HANG", k->i + 1)
    : actor_bitmap_dx (k, "KID", "HANG", k->i);
  k->fo.dy = (k->reverse) ? -actor_bitmap_dy (k, "KID", "HANG", k->i + 1)
    : actor_bitmap_dy (k, "KID", "HANG", k->i);

  if (k->oaction == kid_hang) k->fo.dx = +0, k->fo.dy = +1;

  if (k->reverse && k->j == 0 && k->i == 0
      && k->wait < 3) k->fo.dy = 0;

  return true;
}

static bool
physics_in (struct actor *k)
{
  return true;
}

static void
physics_out (struct actor *k)
{
  struct pos hanged_pos;

  /* depressible floors */
  clear_depressible_floor (k);
  get_hanged_pos (&k->hang_pos, k->f.dir, &hanged_pos);
  press_depressible_floor (&hanged_pos, k);
}
