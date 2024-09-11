from setuptools import setup, find_packages

# get the dependencies and installs
with open("requirements.txt", encoding="utf-8") as f:
    requires = []
    for line in f:
        req = line.split("#", 1)[0].strip()
        if req and not req.startswith("--"):
            requires.append(req)

setup(
    name="gcp-service-account-utils",
    version="0.0.1",
    packages=find_packages(exclude=["tests"]),
    install_requires=requires,
)
