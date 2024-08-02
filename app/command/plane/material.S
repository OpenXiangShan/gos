/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

.section .rodata
.global my_plane
.global end_my_plane

.global ground
.global end_ground

.global bullet1
.global end_bullet1

.global enemy1
.global end_enemy1

my_plane:
	.incbin "app/command/plane/plane.bin"
	.byte 0
end_my_plane:

ground:
	.incbin "app/command/plane/ground.bin"
	.byte 0
end_ground:

bullet1:
	.incbin "app/command/plane/bullet1.bin"
	.byte 0
end_bullet1:

enemy1:
	.incbin "app/command/plane/enemy.bin"
	.byte 0
end_enemy1:
