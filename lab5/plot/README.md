# Plots

## Getting Started

We use [Poetry](https://python-poetry.org/) to manage the dependencies and
environment. Run the following command to setup developing environment.

```sh
poetry install --no-root
```

Remember to activate the virtual environment if not automatically loaded.

```sh
source ./.venv/bin/activate
```

## Usage

After the logging files has been generated, show the line chart with the
following command.

```sh
python plot.py <logging-file>
```

For example,

```sh
python plot.py ../v1/logfilesrv1
```
