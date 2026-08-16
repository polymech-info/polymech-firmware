#!/bin/bash

TARGET_BRANCH=$1

if [ -z "$TARGET_BRANCH" ]; then
  echo "Error: Target branch name must be provided."
  echo "Usage: $0 <target-branch-name>"
  exit 1
fi

echo ">>> Merging from master into $TARGET_BRANCH and updating submodules..."

echo ""
echo ">>> Step 1: Checking out master and pulling latest changes (recursive)..."
git checkout master
if [ $? -ne 0 ]; then
  echo "!!! Error: Failed to checkout master."
  exit 1
fi
git pull --recurse-submodules
if [ $? -ne 0 ]; then
  echo "!!! Error: Failed to pull master with submodules."
  exit 1
fi

echo ""
echo ">>> Step 2: Checking out target branch '$TARGET_BRANCH'..."
git checkout "$TARGET_BRANCH"
if [ $? -ne 0 ]; then
  echo "!!! Error: Failed to checkout target branch '$TARGET_BRANCH'."
  echo "Make sure the branch exists locally. You might need to fetch and checkout first:"
  echo "git fetch origin"
  echo "git checkout $TARGET_BRANCH"
  exit 1
fi

# Optional: It's good practice to ensure the target branch is also up-to-date
# with its remote before merging master into it.
# echo ""
# echo ">>> (Optional) Step 2b: Pulling latest changes for '$TARGET_BRANCH' (recursive)..."
# git pull --recurse-submodules
# if [ $? -ne 0 ]; then
#   echo "!!! Warning: Failed to pull '$TARGET_BRANCH' with submodules. Continuing with merge..."
# fi

echo ""
echo ">>> Step 3: Merging master into '$TARGET_BRANCH'..."
git merge master
if [ $? -ne 0 ]; then
  echo "!!! Error: Merge failed. This could be due to conflicts."
  echo "Please resolve any conflicts manually, then commit the changes."
  echo "After resolving conflicts and committing, you may need to manually run:"
  echo "git submodule update --init --recursive"
  exit 1
fi

echo ""
echo ">>> Step 4: Updating submodules recursively..."
git submodule update --init --recursive
if [ $? -ne 0 ]; then
  echo "!!! Error: Failed to update submodules after merge."
  exit 1
fi

echo ""
echo ">>> Successfully merged master into $TARGET_BRANCH and updated submodules."

echo ""
echo ">>> Step 5: Switching back to master branch..."
git checkout master
if [ $? -ne 0 ]; then
  echo "!!! Warning: Failed to switch back to master branch automatically."
  # Not exiting with error, as the main task was completed.
fi
