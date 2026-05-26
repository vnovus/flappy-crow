# Flappy Crow

Simple implementation of Flappy Bird in C using `raylib`. Code in this
repository is intentionally simple, so it's easier to follow and understand
for beginners.

> Debug mode for visible colliders can be enabled by pressing `[GRAVE]`.

## Compilation

There is no real dependency except raylib, provided `CMakeLists.txt` file
expects `raylib` to be in your path. Project should be able to compile with
the following command.

```shell
$ cmake -S . -B build; cmake --build build
$ ./build/out
```

## External Assets

- **Bird Sprite:** [ma9ici4n](https://ma9ici4n.itch.io/pixel-art-bird-16x16)
