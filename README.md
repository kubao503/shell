# Shell

## Prerequisites
- CMake 3.22
- Flex 2.6
- Bison 3.8

## Building
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Running
```bash
cd build
./shell
```

## Running tests
```bash
cd build
./tests/test
```

## Running Python tests

Python >= 3.10 and Docker is required

Environment setup:

```shell
cd integration_tests
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Running tests:

```shell
cd integration_tests
pytest
```