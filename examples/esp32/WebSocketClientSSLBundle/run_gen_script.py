Import("env")

env.Execute("$PYTHONEXE -m pip install cryptography")
env.Execute("$PYTHONEXE gen_crt_bundle.py --input cacrt_all.pem")