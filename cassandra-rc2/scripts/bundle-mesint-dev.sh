sh scripts/create-bundle.sh
sh scripts/build-deps.sh

cp ./clients/mesint/master/* ./dist/data/
osr-sync zip --source=./dist/ --target=./clients/mesint/master.zip --cwd=./dist --profile=./sync-clients.json --verbose --debug
