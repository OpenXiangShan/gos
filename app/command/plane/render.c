#include "render.h"
#include "mm.h"
#include "asm/type.h"
#include "string.h"
#include "print.h"

static struct renderer renderer;

static void layer_draw_to(struct renderer *renderer, struct layer *p_layer)
{
	int x, y;
	char *out = renderer->draw_board;

	for (y = 0; y < p_layer->height; y++) {
		for (x = 0; x < p_layer->width; x++) {
			char *dst, *src;
			int dst_x, dst_y;
			dst_x = p_layer->pos.X + x;
			dst_y = p_layer->pos.Y + y;

			src =
			    (char *)((unsigned long)p_layer->pattern +
				     y * p_layer->width + x);
			dst =
			    (char *)((unsigned long)out +
				     dst_y * renderer->width + dst_x);

			if (*src == 0 || *src == '\n')
				continue;

			*dst = *src;
		}
	}
}

static void render_clear_all(struct renderer *renderer)
{
	int i;

	print("\033c");

	memset(renderer->draw_board, ' ', renderer->width * renderer->height);
	for (i = 0; i < renderer->height; i++)
		*(char *)((unsigned long)renderer->draw_board +
			  i * renderer->width + (renderer->width - 1)) = '\n';
}

struct layer *get_layer(char *name)
{
	struct layer *p_layer;
	int flags;

	spin_lock_irqsave(&renderer.lock, flags);
	list_for_each_entry(p_layer, &renderer.layers, list) {
		if (!strcmp(p_layer->name, name)) {
			spin_unlock_irqrestore(&renderer.lock, flags);
			return p_layer;
		}
	}
	spin_unlock_irqrestore(&renderer.lock, flags);

	return NULL;
}

void layer_two_dim_process_batch(char *name1, char *name2,
				 int (*process)(struct layer * layer1,
						struct layer * layer2,
						struct layer ** q1,
						struct layer ** q2, int *n1,
						int *n2, void *data),
				 void *data)
{
	struct layer *p_layer1;
	struct layer *p_layer2;
	struct layer *del_q1[128];
	struct layer *del_q2[128];
	int flags, n1 = 0, n2 = 0, i;

	if (!process)
		return;

again:
	n1 = 0;
	n2 = 0;
	spin_lock_irqsave(&renderer.lock, flags);
	list_for_each_entry(p_layer1, &renderer.layers, list) {
		if (!strcmp(p_layer1->name, name1)) {
			list_for_each_entry(p_layer2, &renderer.layers, list) {
				if (!strcmp(p_layer2->name, name2)) {
					if (process(p_layer1, p_layer2,
						    del_q1, del_q2, &n1,
						    &n2, data)) {
						goto free;
					}
				}
			}
		}
	}
free:
	for (i = 0; i < n1; i++) {
		list_del(&del_q1[i]->list);
		mm_free(del_q1[i], sizeof(struct layer));
		mm_free(del_q1[i]->pattern,
			del_q1[i]->width * del_q1[i]->height);
	}
	for (i = 0; i < n2; i++) {
		list_del(&del_q2[i]->list);
		mm_free(del_q2[i], sizeof(struct layer));
		mm_free(del_q2[i]->pattern,
			del_q2[i]->width * del_q2[i]->height);
	}

	if (n1 == 128) {
		spin_unlock_irqrestore(&renderer.lock, flags);
		goto again;
	}
	if (n2 == 128) {
		spin_unlock_irqrestore(&renderer.lock, flags);
		goto again;
	}
	spin_unlock_irqrestore(&renderer.lock, flags);
}

void layer_process_batch(char *name,
			 void (*process)(struct layer * layer,
					 struct layer ** q, int *n, void *data),
			 void *data)
{
	struct layer *p_layer;
	struct layer *del_q[128];
	int flags, n = 0, i;

	if (!process)
		return;

again:
	n = 0;
	spin_lock_irqsave(&renderer.lock, flags);
	list_for_each_entry(p_layer, &renderer.layers, list) {
		if (!strcmp(p_layer->name, name)) {
			process(p_layer, del_q, &n, data);
			if (n == 128)
				goto free;
		}
	}
free:
	for (i = 0; i < n; i++) {
		list_del(&del_q[i]->list);
		mm_free(del_q[i], sizeof(struct layer));
		mm_free(del_q[i]->pattern, del_q[i]->width * del_q[i]->height);
		if (n == 128) {
			spin_unlock_irqrestore(&renderer.lock, flags);
			goto again;
		}
	}
	spin_unlock_irqrestore(&renderer.lock, flags);
}

void layer_release_all(void)
{
	struct layer *p_layer;
	struct layer *del_q[128];
	int n = 0;
	int flags;

again:
	spin_lock_irqsave(&renderer.lock, flags);
	list_for_each_entry(p_layer, &renderer.layers, list) {
		del_q[n++] = p_layer;
		if (n == 128)
			goto free;
	}
free:
	for (int i = 0; i < n; i++) {
		list_del(&del_q[i]->list);
		mm_free(del_q[i], sizeof(struct layer));
		mm_free(del_q[i]->pattern, del_q[i]->width * del_q[i]->height);
		if (n == 128) {
			spin_unlock_irqrestore(&renderer.lock, flags);
			goto again;
		}
	}
	spin_unlock_irqrestore(&renderer.lock, flags);
}

struct layer *generate_layer(char *name, const char *p,
			     int width, int height, int order)
{
	char *buf;
	struct layer *layer;
	int size = width * height;

	layer = (struct layer *)mm_alloc(sizeof(struct layer));
	if (!layer)
		return NULL;

	buf = (char *)mm_alloc(size);
	if (!buf)
		return NULL;

	memcpy((char *)buf, (char *)p, size);

	strcpy((char *)layer->name, name);
	layer->order = order;
	layer->pattern = buf;
	layer->width = width;
	layer->height = height;

	return layer;
}

void render_add_layer(struct layer *layer, int pos_x, int pos_y)
{
	struct layer *p_layer;
	int flags;

	if (!layer)
		return;

	layer->pos.X = pos_x;
	layer->pos.Y = pos_y;

	spin_lock_irqsave(&renderer.lock, flags);
	list_for_each_entry(p_layer, &renderer.layers, list) {
		if (layer->order < p_layer->order) {
			list_add(&layer->list, &p_layer->list);
			spin_unlock_irqrestore(&renderer.lock, flags);
			return;
		}
	}
	list_add_tail(&layer->list, &renderer.layers);
	spin_unlock_irqrestore(&renderer.lock, flags);
}

void render_update(void)
{
	struct layer *p_layer;
	int flags;

	render_clear_all(&renderer);

	spin_lock_irqsave(&renderer.lock, flags);
	list_for_each_entry(p_layer, &renderer.layers, list) {
		layer_draw_to(&renderer, p_layer);
	}
	spin_unlock_irqrestore(&renderer.lock, flags);

	print("%s", renderer.draw_board);
}

int render_init(int width, int height)
{
	char *buf;

	buf = (char *)mm_alloc(width * height);
	if (!buf)
		return -1;

	renderer.width = width;
	renderer.height = height;
	renderer.draw_board = buf;

	INIT_LIST_HEAD(&renderer.layers);
	render_clear_all(&renderer);

	return 0;
}
