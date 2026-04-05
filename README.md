JCUnit - a very simple unit testing framework for C
===

## Installation

Build and install to `/usr/local/bin`:

```sh
make install
```

To build without installing:

```sh
make
```

## Getting Started

Write a test file (e.g. `example.test`):

```text
@test("echo prints hello")
@given("file")
hello

@whenRun("/bin/echo", args="hello")
@expectOutput("stdout")
hello

@endtest
```

Run it:

```sh
jcunit example.test
```

You can also pass a directory to run all `.test` files in it:

```sh
jcunit ./tests
```

Output:

```text
Test: example.test
	PASS       echo prints hello

Passed: 1, Failed: 0
```

## Usage

```
jcunit [options] path [paths...]
```

### Options

| Option | Description |
|---|---|
| `--run-mode=(detail\|passthrough)` | Output mode (default: `passthrough`) |
| `--colors` | Enable colored diff output |
| `--no-cache` | Disable cache for parsing and assembling phases |
| `--clear-cache` | Clear the cache file before running |
| `--version` | Show version |
| `--help` | Show help message |

Caching can also be disabled by setting the environment variable `JCUNIT_NO_CACHE=1`.

## Test Language

A test file contains one or more tests. Each test has:

- `@test("name")` — test declaration
- `@given("file")` — input file content (follows on next lines)
- `@given("none")` — no input file; the program is invoked without a file argument
- `@given("reference", path="path/to/file.txt")` — use an existing file; the path is passed as-is as the first argument
- `@whenRun("./program")` — program to execute (with the given file as first argument if `@given("file")` or `@given("reference")`)
- `@expectOutput("stdout")` or `@expectOutput("stderr")` — expected output (follows on next lines)
- `@endtest` — end of test

Use `@given("none")` for testing programs that don't need input files (e.g. `--help` output).
Use `@given("reference", path="...")` to test with an existing file without inlining its content.

A test with `@shouldBeSkipped` is **skipped**.

See the full grammar in [test-grammar](./test-grammar) and examples in the [examples/](./examples/) directory.

## About

JCUnit is an Open Source project covered by the [MIT License](./LICENSE).

The name "JCUnit" is just an abbreviation of "Just C Unit".
