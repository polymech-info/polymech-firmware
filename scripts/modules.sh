
## remove submodule
git submodule deinit -f cassandra-rc2/lib/polymechg-base
git rm -f cassandra-rc2/lib/polymechg-base
git commit -m "Remove submodule"
git push
