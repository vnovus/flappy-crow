#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>

#include "raylib.h"
#include "raymath.h"

// =============================================
// Built-in Types
// =============================================

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

typedef intptr_t  iptr;
typedef uintptr_t uptr;
typedef size_t    usize;

#define VECTOR2_ZERO ((Vector2) {0.f, 0.f})

// =============================================
// Type Declerations
// =============================================

// These are only declared so project can remain as a single file.

typedef struct AnimatedTexture AnimatedTexture;
typedef struct Pipe            Pipe;
typedef struct Bird            Bird;
typedef struct State           State;

// =============================================
// Game Constants
// =============================================

// Screen

const i32 SCREEN_WIDTH  = 800;
const i32 SCREEN_HEIGHT = 600;

// Font Sizes

const i32 FONT_SIZE = 32;

// =============================================
// AnimatedTexture
// =============================================

struct AnimatedTexture {
  Texture   texture;
  usize     size[2];
  usize     frames;
  usize     current;
  f64       time;
  f64       frame_time;
};

AnimatedTexture
animated_texture_init(const char *path, usize fx, usize fy, usize fps);

void
animated_texture_deinit(AnimatedTexture *self);

void
animated_texture_update(AnimatedTexture *self);

void
animated_texture_render(const AnimatedTexture *self,
                        Vector2                position,
                        Vector2                size,
                        f32                    rotation);

// =============================================
// Pipe
// =============================================

#define PIPE_COUNT 5

const f32 PIPE_WIDTH         = 96.f / 2.0f;
const f32 PIPE_HEIGHT        = 172.f / 2.0f;
const f32 PIPE_INTERVAL      = 256.f; 
const f32 PIPE_PADDING       = 64.f;
const f32 PIPE_INITIAL_SPEED = 128.0f;

struct Pipe {
  Vector2 position;
  bool    expended;
};

void
pipe_setup(Pipe* self, Vector2 position);

void
pipe_update(Pipe *self, const State *state);

void
pipe_render(const Pipe *self, const State *state);

void
pipe_respawn(Pipe *self, const State *state);

// =============================================
// Bird
// =============================================

const f32 BIRD_SPRITE_SIZE   = 64.f;
const f32 BIRD_WIDTH         = 64.f;
const f32 BIRD_HEIGHT        = 32.f;
const f32 BIRD_JUMP_FORCE    = 228.0f;
const f32 BIRD_JUMP_TIME     = 0.3f;
const f32 BIRD_TERM_VELOCITY = 640.0f;

struct Bird {
  bool            alive;
  Vector2         position;
  Vector2         size;
  Vector2         velocity;
  f32             rotation;
  f64             jump_time;
  AnimatedTexture texture;
};

Bird
bird_init(Vector2 position);

void
bird_deinit(Bird *self);

void
bird_update(Bird *self, State *state);

void
bird_render(const Bird *self, const State *state);

void
bird_input(Bird *self, State *state);

void
bird_apply_gravity(Bird *self);

bool
bird_check_collision(Bird *self, const Pipe *pipe);

void
bird_check_score(Bird *self, State *state, Pipe *pipe);

// =============================================
// State
// =============================================

struct State {
  // Game State
  bool pause;
  u32  score;
  bool debug;
  // Bird
  Bird bird;
  // Pipes
  Pipe pipes[PIPE_COUNT];
  f32  pipe_speed;
};

State
state_init();

void
state_deinit(State *self);

void
state_update(State *self);

void
state_render(const State *self);

void
state_setup_pipes(State *self);

// =============================================
// Main
// =============================================

int
main() {

  srand(time(0));

  SetTraceLogLevel(LOG_WARNING);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Flappy Crow");
  SetTargetFPS(60);

  State state = state_init();
  f64   delta = 0.;

  while (!WindowShouldClose()) {

    state_update(&state);

    BeginDrawing();
    ClearBackground(BLUE);
    state_render(&state);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}

// =============================================
// AnimatedTexture - Impl
// =============================================

AnimatedTexture
animated_texture_init(const char *path, usize fx, usize fy, usize fps) {
  
  AnimatedTexture t = {0};

  t.texture    = LoadTexture(path);
  t.size[0]    = fx;
  t.size[1]    = fy;
  t.frames     = fx * fy;
  t.current    = 0;
  t.frame_time = 1.0 / (f64)fps;
  t.time       = 0.;

  return t;
}

void
animated_texture_deinit(AnimatedTexture *self) {
  
  UnloadTexture(self->texture);
}

void
animated_texture_update(AnimatedTexture *self) {

  f64 time = GetTime();

  if (time < self->time) {
    return;
  }

  self->time    = time + self->frame_time;
  self->current = (self->current + 1) % self->frames;
}

void
animated_texture_render(const AnimatedTexture *self,
                        Vector2                position,
                        Vector2                size,
                        f32                    rotation) {

  i32 x = self->current        % self->size[0];
  i32 y = self->current        / self->size[1];
  f32 w = self->texture.width  / (f32)self->size[0];
  f32 h = self->texture.height / (f32)self->size[1];

  Rectangle source = {
    .x      = x * w,
    .y      = y * h,
    .width  = w,
    .height = h,
  };

  Rectangle dest = {
    .x      = position.x,
    .y      = position.y,
    .width  = size.x == 0 ? w : size.x,
    .height = size.y == 0 ? h : size.y,
  };

  DrawTexturePro(
    self->texture,
    source,
    dest,
    (Vector2) {size.x / 2., size.y / 2.},
    rotation,
    WHITE
  );
}

// =============================================
// Pipe - Impl
// =============================================

void
pipe_setup(Pipe * self, Vector2 position) {

  self->position = position;
  self->expended = false;
}

void
pipe_update(Pipe *self, const State *state) {

  if (state->pause) {
    return;
  }

  if (self->position.x + PIPE_WIDTH <= 0.f) {
    pipe_respawn(self, state);
    return;
  }

  self->position.x -= state->pipe_speed * GetFrameTime();
}

void
pipe_render(const Pipe *self, const State *state) {

  DrawRectangle(
    self->position.x - PIPE_WIDTH,
    self->position.y - PIPE_HEIGHT - SCREEN_HEIGHT,
    PIPE_WIDTH * 2.f,
    SCREEN_HEIGHT,
    GREEN);

  DrawRectangle(
    self->position.x - PIPE_WIDTH,
    self->position.y + PIPE_HEIGHT,
    PIPE_WIDTH * 2.f,
    SCREEN_HEIGHT,
    GREEN);

  if (state->debug) {
    DrawRectangleLines(
      self->position.x - PIPE_WIDTH,
      self->position.y - PIPE_HEIGHT - SCREEN_HEIGHT,
      PIPE_WIDTH * 2.f,
      SCREEN_HEIGHT,
      RED);

    DrawRectangleLines(
      self->position.x - PIPE_WIDTH,
      self->position.y + PIPE_HEIGHT,
      PIPE_WIDTH * 2.f,
      SCREEN_HEIGHT,
      RED);
  }
}

void
pipe_respawn(Pipe *self, const State *state) {

  const Pipe *next = &state->pipes[0];
  for (const Pipe *pipe = state->pipes; pipe < (state->pipes + PIPE_COUNT); ++pipe) {

    if (pipe == self) {
      continue;
    }

    if (pipe->position.x > next->position.x) {
      next = pipe;
    }
  }

  i32 pady = PIPE_HEIGHT + PIPE_PADDING;
  i32 rndy = SCREEN_HEIGHT - (pady * 2.0);
  i32 posx = PIPE_INTERVAL + (PIPE_WIDTH * 2.0);

  self->position.x = next->position.x + posx;
  self->position.y = (rand() % rndy) + pady;
  self->expended   = false;
}

// =============================================
// Bird - Impl
// =============================================

Bird
bird_init(Vector2 position) {

  Bird b = {0};

  b.alive     = true;
  b.position  = position;
  b.size      = (Vector2) {BIRD_WIDTH, BIRD_HEIGHT};
  b.velocity  = (Vector2) {PIPE_INITIAL_SPEED, 0.f};
  b.rotation  = 0.f;
  b.jump_time = 0.f;
  b.texture   = animated_texture_init("res/bird.png", 8, 1, 12);

  return b;
}

void
bird_deinit(Bird *self) {
  
  animated_texture_deinit(&self->texture);
}

void
bird_render(const Bird *self, const State *state) {

  const Vector2 sprite_size = { BIRD_SPRITE_SIZE, BIRD_SPRITE_SIZE };
  
  animated_texture_render(
    &self->texture,
    self->position,
    sprite_size,
    self->rotation * RAD2DEG / 2.f);

  if (state->debug) {
    DrawRectangleLines(
      self->position.x - (self->size.x / 2.f),
      self->position.y - (self->size.y / 2.f),
      self->size.x,
      self->size.y,
      RED);
  }
}

void
bird_update(Bird *self, State *state) {

  bird_input(self, state);

  if (!state->pause) {

    // Calculate the rotation for the bird's sprite.
    Vector2 normalized = Vector2Normalize(self->velocity);

    if (self->alive) {
      self->rotation = atan2f(normalized.y, normalized.x);
    } else {
      self->rotation += PI * GetFrameTime();
    }

    bird_apply_gravity(self);
  }

  // End the game when the bird hit's the lower end of the screen.
  if (self->position.y + (self->size.y / 2.f) > SCREEN_HEIGHT) {

    self->position.y = SCREEN_HEIGHT - (self->size.y / 2.f);
    state->pause     = true;
    self->alive      = false;
  }

  if (!self->alive) {
    return;
  }

  animated_texture_update(&self->texture);

  for (usize i = 0; i < PIPE_COUNT; ++i) {

    self->alive = !bird_check_collision(self, &state->pipes[i]);
    if (!self->alive) {
      return;
    }

    bird_check_score(self, state, &state->pipes[i]);
  }
}

void
bird_input(Bird *self, State *state) {
  
  if (!self->alive) {
    return;
  }

  if (IsKeyPressed(KEY_SPACE)) {

    self->velocity.y = -BIRD_JUMP_FORCE;
    self->jump_time  = GetTime() + BIRD_JUMP_TIME; 

    if (state->pause) {
      state->pause = false;
    }
  }
}

void
bird_apply_gravity(Bird *self) {

  self->position.y += self->velocity.y * GetFrameTime();

  if (GetTime() < self->jump_time) {

    self->velocity.y *= 0.95f;
    return;
  } 

  f32 vdiff = BIRD_TERM_VELOCITY - self->velocity.y;
  self->velocity.y += vdiff * 0.9 * GetFrameTime();
  if (self->velocity.y > BIRD_TERM_VELOCITY) {
    self->velocity.y = BIRD_TERM_VELOCITY;
  }
}

bool
bird_check_collision(Bird *self, const Pipe *pipe) {

  bool colx = (self->position.x - (self->size.x / 2.f) < pipe->position.x + PIPE_WIDTH) &&
              (self->position.x + (self->size.x / 2.f) > pipe->position.x - PIPE_WIDTH);
  bool coly = (self->position.y + (self->size.y / 2.f) > pipe->position.y + PIPE_HEIGHT) ||
              (self->position.y - (self->size.y / 2.f) < pipe->position.y - PIPE_HEIGHT);

  // Returns true if we are in the zone of a pipe, but not inside it's empty part.
  return colx && coly;
}

void
bird_check_score(Bird *self, State *state, Pipe *pipe) {

  if (self->position.x > pipe->position.x && !pipe->expended) {

    pipe->expended = 1;
    state->score  += 1;
  }
}

// =============================================
// State - Impl
// =============================================

State
state_init() {

  State s = {0};

  // Game State
  s.pause = true;
  s.score = 0;
  s.debug = false;

  // Bird
  s.bird = bird_init((Vector2) {
    SCREEN_WIDTH  / 2.f,
    SCREEN_HEIGHT / 2.f
  });

  // Pipes
  s.pipe_speed = PIPE_INITIAL_SPEED;
  state_setup_pipes(&s);

  return s;
}

void
state_deinit(State *self) {

  bird_deinit(&self->bird);
}

void
state_update(State *self) {

  if (IsKeyPressed(KEY_GRAVE)) {
    self->debug = !self->debug;
  }

  for (usize i = 0; i < PIPE_COUNT; ++i) {
    pipe_update(&self->pipes[i], self);
  }

  bird_update(&self->bird, self);

  // Restart the game if the bird is dead.
  if (self->bird.alive || !self->pause) {
    return;
  }
  if (IsKeyPressed(KEY_R)) {
    (*self) = state_init();
  }
}

void
state_render(const State *self) {

  for (usize i = 0; i < PIPE_COUNT; ++i) {
    pipe_render(&self->pipes[i], self);
  }

  bird_render(&self->bird, self);

  // This measurments can easily be cached but this is flappy bird so.
  const char *promt = TextFormat("%d", self->score);
  int         width = MeasureText(promt, (FONT_SIZE * 2));
  DrawText(promt, ((SCREEN_WIDTH - width) / 2.f), (FONT_SIZE * 2), 64, BLACK);

  // Start or Restart text.
  if (self->pause) {

    const char *promt = (self->bird.alive) ? "Press [Space] to start." : "Press [R] to restart.";
    int         width = MeasureText(promt, FONT_SIZE);

    DrawText(promt, ((SCREEN_WIDTH - width) / 2.f), SCREEN_HEIGHT / 2.f, FONT_SIZE, WHITE);
  }
}

void
state_setup_pipes(State *self) {

  i32 pady = PIPE_HEIGHT + PIPE_PADDING;
  i32 rndy = SCREEN_HEIGHT - (pady * 2.0);
  i32 posx = SCREEN_WIDTH;

  for (usize i = 0; i < PIPE_COUNT; ++i) {

    pipe_setup(&self->pipes[i], (Vector2) {
      posx,
      (rand() % rndy) + pady,
    });
    
    posx += PIPE_INTERVAL + (PIPE_WIDTH * 2);
  }
}
