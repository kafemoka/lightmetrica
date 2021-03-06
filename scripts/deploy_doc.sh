#!/bin/bash
cd $HOME
git config --global user.email "travis@travis-ci.org"
git config --global user.name "Travis"
git clone --quiet --branch=gh-pages https://${GH_TOKEN}@github.com/hi2p-perim/lightmetrica.git gh-pages > /dev/null
cd gh-pages
cp -rf $TRAVIS_BUILD_DIR/build/doc/html/* .
git add -f .
git commit -m "Travis build $TRAVIS_BUILD_NUMBER pushed to gh-pages"
git push --force --quiet origin gh-pages > /dev/null