import os


def before_all(context):
    context.example_binary = os.environ.get(
        "EXAMPLE_BINARY", "build/debug/Example/ExampleProgram"
    )
