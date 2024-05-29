#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../../command.h"
#include "render.h"
#include "event_loop.h"
#include "timer.h"
#include "event.h"
#include "string.h"
#include "collision.h"
#include "task.h"
#include "plane.h"

#define SCREEN_WIDTH 69
#define SCREEN_HEIGHT 44

extern const char my_plane[];
extern unsigned long end_my_plane;
extern const char ground[];
extern unsigned long end_ground[];
extern const char bullet1[];
extern unsigned long end_bullet1;
extern const char enemy1[];
extern unsigned long end_enemy1[];

static unsigned long frame_cnt = 0;

static struct materal materal[] = {
//      { "ground", 69, 42, ground, 0, 0 },
	{ "comm_bullet", 2, 1, bullet1, 36, 36, 1, 1 },
	{ "my_plane", 6, 4, my_plane, 34, 41 - 4, 0, 1 },
	{ "enemy1", 4, 2, enemy1, 0, 0, 1, 5 }
};

#define MATERAL_COUNT (sizeof(materal) / sizeof(materal[0]))

static unsigned long seed;
static unsigned int get_random(int from, int to)
{
	unsigned int ran;

	if (from > to) {
		int ch;

		ch = from;
		from = to;
		to = ch;
	}

	seed = seed * 1103515245 + 12345;
	ran = (seed / 65536) % 32768;

	return ran % (to - from) + from;
}

static struct materal *get_materal(char *name)
{
	int i;

	for (i = 0; i < MATERAL_COUNT; i++) {
		if (!strcmp(materal[i].name, name))
			return &materal[i];
	}

	return NULL;
}

static int collision_update(struct layer *layer1, struct layer *layer2,
			    struct layer **q1, struct layer **q2,
			    int *n1, int *n2, void *data)
{
	int nn_bullet = *n1;
	int nn_enemy = *n2;
	struct layer *bullet = layer1;
	struct layer *enemy = layer2;

	if (check_collision(bullet->width, bullet->height,
			    bullet->pos.X, bullet->pos.Y,
			    enemy->width, enemy->height,
			    enemy->pos.X, enemy->pos.Y)) {
		q1[nn_bullet++] = bullet;
		q2[nn_enemy++] = enemy;

		*n1 = nn_bullet;
		*n2 = nn_enemy;
		return 1;
	}

	return 0;
}

static void bullet_enemy_collision_update(void)
{
	struct materal *m_bullet = get_materal("comm_bullet");
	struct materal *m_enemy = get_materal("enemy1");

	layer_two_dim_process_batch(m_bullet->name, m_enemy->name,
				    collision_update, NULL);
}

static void bullet_auto_update(struct layer *layer, struct layer **q,
			       int *n, void *data)
{
	int nn = *n;
	struct materal *m = get_materal("comm_bullet");

	layer->pos.Y -= m->speed;
	if (layer->pos.Y <= 0)
		q[nn++] = layer;

	*n = nn;
}

static void bullet_update(void)
{
	struct materal *m;
	struct layer *layer;
	struct layer *my_plane;

	m = get_materal("comm_bullet");

	if (frame_cnt % m->speed_div)
		return;

	my_plane = get_layer("my_plane");

	layer_process_batch(m->name, bullet_auto_update, NULL);

	layer = generate_layer(m->name, m->pattern, m->width, m->height, 0);
	if (!layer)
		return;

	render_add_layer(layer,
			 my_plane->pos.X + my_plane->width / 2 - 1,
			 my_plane->pos.Y - 1);
}

static void enemy_auto_update(struct layer *layer, struct layer **q,
			      int *n, void *data)
{
	int nn = *n;
	struct materal *m = get_materal("enemy1");

	layer->pos.Y += m->speed;
	if (layer->pos.Y >= SCREEN_HEIGHT)
		q[nn++] = layer;

	*n = nn;
}

static void enemy_update(void)
{
	struct materal *m;
	struct layer *layer;
	int pos;

	m = get_materal("enemy1");

	if (frame_cnt % m->speed_div)
		return;

	layer_process_batch(m->name, enemy_auto_update, NULL);

	layer = generate_layer(m->name, m->pattern, m->width, m->height, 0);
	if (!layer)
		return;

	pos = get_random(5, SCREEN_WIDTH - 5);

	render_add_layer(layer, pos, 0);
}

static void plane_render_timer_handler(void *data)
{
	frame_cnt++;

	bullet_update();
	enemy_update();
	bullet_enemy_collision_update();
	render_update();
}

static int event_process(char command)
{
	struct materal *me = get_materal("my_plane");
	struct layer *my_plane_layer = get_layer("my_plane");

	if (command == 'w') {
		if (check_position_valid(SCREEN_WIDTH, SCREEN_HEIGHT,
					 my_plane_layer->pos.X,
					 my_plane_layer->pos.Y,
					 me->width, me->height, 'w')) {
			my_plane_layer->pos.Y -= 1;
		}
	} else if (command == 's') {
		if (check_position_valid(SCREEN_WIDTH, SCREEN_HEIGHT,
					 my_plane_layer->pos.X,
					 my_plane_layer->pos.Y,
					 me->width, me->height, 's')) {
			my_plane_layer->pos.Y += 1;
		}
	} else if (command == 'a') {
		if (check_position_valid(SCREEN_WIDTH, SCREEN_HEIGHT,
					 my_plane_layer->pos.X,
					 my_plane_layer->pos.Y,
					 me->width, me->height, 'a')) {
			my_plane_layer->pos.X -= 1;
		}
	} else if (command == 'd') {
		if (check_position_valid(SCREEN_WIDTH, SCREEN_HEIGHT,
					 my_plane_layer->pos.X,
					 my_plane_layer->pos.Y,
					 me->width, me->height, 'd')) {
			my_plane_layer->pos.X += 1;
		}
	} else if (command == 'q')
		return -1;

	return 0;
}

static int cmd_plane_handler(int argc, char *argv[], void *priv)
{
	struct layer *layer;
	struct materal *m = get_materal("my_plane");
	struct timer_event_info *timer;

	frame_cnt = 0;

	render_init(SCREEN_WIDTH, SCREEN_HEIGHT);

	layer = generate_layer(m->name, m->pattern, m->width, m->height, 0);
	render_add_layer(layer, m->x, m->y);

	timer = set_timer_restart_cpu(50, plane_render_timer_handler, NULL, 1);

	event_loop_start(event_process);

	//cmd_plane_end
	del_timer_cpu(timer, 1);
	wait_for_ms(1000);
	layer_release_all();

	return 0;
}

static const struct command cmd_plane = {
	.cmd = "plane",
	.handler = cmd_plane_handler,
	.priv = NULL,
};

int cmd_plane_init()
{
	register_command(&cmd_plane);

	return 0;
}

APP_COMMAND_REGISTER(plane, cmd_plane_init);
