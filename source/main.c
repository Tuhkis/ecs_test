#define SR_IMPL
#include "sr/simple_renderer.h"
#include "glfw_ez/glfw-ez.c"

#include "time.h"

#define APP_WIDTH (800)
#define APP_HEIGHT (800)
#define APP_TITLE ((char*)"Ecs Test")
#define APP_FPS (60)

#define MAX_ENTITIES (64)

#define entity_add_component(ecs, id, component_type) \
  ecs.component_type##_arr[id].exist = 1

#define entity_get_component(ecs, e, type) \
  &ecs.type##_arr[e]

#define rand_range(a, b) a + (rand() % (b - a))

typedef unsigned int Entity;

typedef struct
{
  char exist;
  sr_Vec2 pos;
  sr_Vec2 size;
} TransformComp;

typedef struct
{
  char exist;
  TransformComp* transform;
  sr_Vec4 colour;
} VisComp;

typedef struct
{
  char exist;
  float lifetime;
  TransformComp* transform;
} MoveComp;

static inline void
vc_tick(VisComp* c, Entity e);

static inline void
tc_tick(TransformComp* c, Entity e);

static inline void
mc_tick(MoveComp* c, Entity e);

#define COMPONENTS \
  X(VisComp, vc_tick) \
  X(TransformComp, tc_tick) \
  X(MoveComp, mc_tick)

typedef struct
{
  char free_ids[MAX_ENTITIES];
#define X(type, tick) type type##_arr[MAX_ENTITIES];
  COMPONENTS
#undef X
} Ecs;

typedef struct
{
  GLFWwindow* win;
  sr_Renderer* rend;
  Ecs ecs;
  unsigned int white_tex;
  float delta_time;
} App;
static App app = {0};

void
ecs_init(Ecs* ecs)
{
  for (Entity e = 0; e < MAX_ENTITIES; ++e)
    ecs->free_ids[e] = 1;
}

static inline Entity
ecs_get_entity(Ecs* ecs)
{
  for (Entity e = 0; e < MAX_ENTITIES; ++e)
    if (ecs->free_ids[e] == 1)
    {
      ecs->free_ids[e] = 0;
      return e;
    }
}

static inline void
ecs_tick(Ecs* ecs)
{
  for (Entity e = 0; e < MAX_ENTITIES; ++e)
  {
#define X(type, tick) tick( &ecs->type##_arr[e], e );
    COMPONENTS
#undef X
  }
}

static inline void
vc_tick(VisComp* c, Entity e)
{
  if (c->exist == 0) { return; }
  else if (c->exist == 1)
  {
    c->transform = entity_get_component(app.ecs, e, TransformComp);
    c->colour = sr_vec4(
        ((float)rand_range(90, 255)) / 255.f,
        ((float)rand_range(100, 255)) / 255.f,
        ((float)rand_range(150, 255)) / 255.f,
        ((float)rand_range(200, 255)) / 255.f
      );
    c->exist = 2;
    return;
  } else
  {
    sr_render_push_quad(app.rend, c->transform->pos, c->transform->size, c->colour, app.white_tex);
    return;
  }
}

static inline void
tc_tick(TransformComp* c, Entity e)
{
  if (c->exist == 0) { return; }
  else if (c->exist == 1)
  {
    c->pos = sr_vec2(rand_range(0, 800), rand_range(0, 800));
    c->size = sr_vec2(rand_range(32, 77), rand_range(32, 99));
    c->exist = 2;
  } else {}
}

static inline void
mc_tick(MoveComp* c, Entity e)
{
  if (c->exist == 0) { return; }
  else if (c->exist == 1)
  {
    c->transform = entity_get_component(app.ecs, e, TransformComp);
    c->exist = 2;
    return;
  } else
  {
    if ((int)c->lifetime % 2 == 0)
      c->transform->pos.x += 1;
    else
      c->transform->pos.x -= 1;

    c->transform->pos.y += sinf(c->lifetime * 32.f) * 3.f;

    c->lifetime += .01f;
    return;
  }
}

int
main()
{
  srand(time(NULL));
  glfwInit();
  app.win = glfwCreateWindow(APP_WIDTH, APP_HEIGHT, APP_TITLE, NULL, NULL);
  glfwMakeContextCurrent(app.win);
  sr_load_loader((void*)glfwGetProcAddress);

  sr_Renderer r = {0};
  sr_init(&r, APP_WIDTH, APP_HEIGHT);
  app.rend = &r;
  glClearColor(.2f, .2f, .2f, 1.f);

  app.white_tex = sr_get_white_texture();

  ecs_init(&app.ecs);
  for (int i = 0; i < MAX_ENTITIES; ++i)
  {
    Entity e = ecs_get_entity(&app.ecs);
    entity_add_component(app.ecs, e, TransformComp);
    entity_add_component(app.ecs, e, VisComp);
    entity_add_component(app.ecs, e, MoveComp);
  }

  while (!glfwWindowShouldClose(app.win) && 1)
  {
    Sleep(1000 / APP_FPS);
    glClear(GL_COLOR_BUFFER_BIT);
    sr_render_begin(&r);
      ecs_tick(&app.ecs);
    sr_render_end(&r);
    glfwPollEvents();
    glfwSwapBuffers(app.win);
  }


  glfwDestroyWindow(app.win);
  glfwTerminate();
  return 0;
}

