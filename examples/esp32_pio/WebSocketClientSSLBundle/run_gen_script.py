Import("env")

env.Execute("$PYTHONEXE -m pip install cryptography")
env.Execute("$PYTHONEXE gen_crt_bundle.py --input cacrt_all.pem")
env.Execute("mkdir -p data/cert")
env.Execute("mv -f x509_crt_bundle data/cert/x509_crt_bundle.bin")
