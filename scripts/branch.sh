branch="cassandra-650-rc3"
#branch="master"
git checkout $branch
git submodule foreach git checkout $branch
git pull --recurse-submodules
