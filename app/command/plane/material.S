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
