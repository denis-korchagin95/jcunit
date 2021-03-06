JCUnit - a very simple unit testing framework for C
===

## Installation

By default, you need only run the "make install" command and "make" do for you all installation routines.

If you want only to build it without installation on your computer just run "make build" command.


## Getting Started

Write a test in the following format:

```text
@test("Test #1")
@given("file")
@@test

@whenRun("./bin/test-tokenizer")
@expectOutput("stdout")
<TOKEN_DIRECTIVE 'test'>
<TOKEN_NEWLINE '\n'>
<TOKEN_EOF>

@endtest

@test("Test #2")
@given("file")
@@test

@whenRun("./bin/test-tokenizer")
@expectOutput("stdout")
<TOKEN_DIRECTIVE 'test'>
<TOKEN_EOF>

@endtest

@test("Test #3")
@given("file")
@@test

@whenRun("./bin/test-tokenizer")
@endtest
```

You can get the full test's language grammar by link [JCUnit Test's Language Grammar](./test-grammar) and some examples in directory `./examples/`

After that, run the JCUnit with that test file `jcunit ./test.test`

It will show you the results of test execution:

```text
Test: test.test
	PASS       Test #1
	FAIL       Test #2
--- Expected
<TOKEN_DIRECTIVE 'test'>
<TOKEN_EOF>
$
+++ Actual
<TOKEN_DIRECTIVE 'test'>
<TOKEN_NEWLINE '\n'>
<TOKEN_EOF>
$
	INCOMPLETE Test #3

Passed: 1, Failed: 1, Incomplete: 1
```

## About

JCUnit is an Open Source project covered by the GNU General Public
License version 2.

The name "JCUnit" it's just an abbreviation from the phrase "Just C Unit".
