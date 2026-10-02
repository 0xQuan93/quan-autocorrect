#!/usr/bin/env python3
import os
import pathlib
import subprocess
import tempfile
import shutil
import sys
root = pathlib.Path(__file__).resolve().parent
with tempfile.TemporaryDirectory(prefix='quan-autocorrect-test-') as d:
    env = os.environ.copy()
    env.update(XDG_CONFIG_HOME=d+'/config', XDG_DATA_HOME=d+'/data', XDG_CACHE_HOME=d+'/cache')
    target = pathlib.Path(d)/"config/enchant/hunspell"
    target.mkdir(parents=True)
    for source in (root/"dictionary").glob("en_US.*"):
        shutil.copy2(source, target/source.name)
    binary = sys.argv[1] if len(sys.argv) > 1 else str(root/'build/test-autocorrect')
    raise SystemExit(subprocess.run([binary], env=env, timeout=25).returncode)
