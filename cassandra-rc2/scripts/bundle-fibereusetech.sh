sh scripts/create-bundle.sh
cp ./clients/fibereusetech/master/* ./dist/data/
osr-sync zip --source=./dist/ --target=./clients/fibereusetech/master.zip --cwd=./dist --profile=./sync-clients.json --verbose --debug
