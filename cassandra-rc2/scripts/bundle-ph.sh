sh scripts/create-bundle.sh
sh scripts/build-deps.sh

cp ./clients/plastichub/master/* ./dist/data/

osr-sync zip --source=./dist/ --target=./clients/plastichub/master.zip --cwd=./dist --profile=./sync-clients.json --verbose --debug
