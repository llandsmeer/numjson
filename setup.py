from setuptools import Extension, setup

setup(
    ext_modules=[
        Extension(
            name="numjson",
            sources=["numjson.cpp"],
        ),
    ]
)
