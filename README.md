# STOKE (Leakage-Aware Extension for Synth∀)

This repository holds Synth∀'s fork of STOKE, a stochastic superoptimizer for x86
assembly.

To learn how STOKE works in general, see the [original STOKE README](STOKE_README.md).
We recommend reading the original README first, since the leakage-aware extensions to
STOKE follow the same fundamental structures.

Synth∀ runs STOKE in a Docker container. The easiest way to do this is through the
scripts found in that repo, i.e., `stoke_setup.sh`, `stoke_teardown.sh`, and
`run_stoke_in_container.sh`/`stoke_bench.sh`.

If you would like to run STOKE manually, `synth/run_stoke.sh` in this repo can be used
to do so. Just make sure to set up and SSH into the Docker container first.
After creating an SSH keypair at `stoke_dockerkey` and `stoke_dockerkey.pub`:

```sh
docker build -t stoke:eval .

# expose the container on port 2000
docker run --name stoke-test -d -it -p 2000:22 stoke:eval

# then SSH in
ssh -i stoke_dockerkey -p 2000 stoke@localhost
```

From there, you can explore the repo, run STOKE, etc.
