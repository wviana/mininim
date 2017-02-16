/*
  torch.c -- torch module;

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

void
draw_torch (ALLEGRO_BITMAP *bitmap, struct pos *p,
            enum em em, enum vm vm)
{
  ALLEGRO_BITMAP *torch = L_bitmap (main_L, "TORCH", 1);
  if (! torch) return;

  torch = L_apply_palette (main_L, torch, "base_palette");

  struct coord c;
  if (! L_coord (main_L, "TORCH", 1, p, &c)) return;

  if (vm == VGA) torch = apply_hue_palette (torch);
  if (peq (p, &mouse_pos))
    torch = apply_palette (torch, selection_palette);

  draw_bitmapc (torch, bitmap, &c, 0);
}
