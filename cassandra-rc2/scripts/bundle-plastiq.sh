sh scripts/create-bundle.sh
sh scripts/build-deps.sh

cp ./clients/plastiq/master/* ./dist/data/
osr-sync zip --source=./dist/ --target=./clients/plastiq/master.zip --cwd=./dist --profile=./sync-clients.json --verbose --debug

cp ./clients/plastiq/slave/* ./dist/data/
osr-sync zip --source=./dist/ --target=./clients/plastiq/slave.zip --cwd=./dist --profile=./sync-clients.json --verbose --debug

