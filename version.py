#!/usr/bin/env python3

import re
import shutil
import subprocess
import sys

MAJOR = 0
MINOR = 0
PATCH = 1
PRE_RELEASE = True

def run(*args):
    return subprocess.run(args, capture_output=True, text=True)

def main():
    has_git = shutil.which('git') is not None

    # Only releases should be used without `git`
    if not has_git:
        # We need `git` to create the pre-release version
        if PRE_RELEASE:
            print(f'git required to determine version', file=sys.stderr)
            exit(1)

        print(f'{MAJOR}.{MINOR}.{PATCH}')
        return

    p = run('git', 'rev-parse', '--short=7', 'HEAD')
    # If we can't get the current revision then we (hopefully) don't have any
    # commits, set the pre-release version to 1.
    if p.returncode != 0:
        print(f'{MAJOR}.{MINOR}.{PATCH}-1+git.dirty')
        return
    commit = p.stdout.strip()

    dirty = run('git', 'diff', '--quiet').returncode != 0

    # Version tags should be the only tags that start with a digit
    VERSION_GLOB = '[0-9]*'

    p = run('git', 'describe', '--exact-match', '--match', VERSION_GLOB, 'HEAD')
    if p.returncode == 0:
        # The current commit has a version tag
        tag = p.stdout.strip()
        mo = re.match(r'(\d+)\.(\d+)\.(\d+)', tag)
        # The tag should only have a major, minor, and patch version
        if mo is None:
            print(f"git tag '{tag}' must match format MAJOR.MINOR.PATCH",
                  file=sys.stderr)
            exit(1)
        tag_major = int(mo.group(1))
        tag_minor = int(mo.group(2))
        tag_patch = int(mo.group(3))
        # The version tag matches the version expected in this file
        if MAJOR == tag_major and MINOR == tag_minor and PATCH == tag_patch:
            # If there are any changes, we should not build, as this doesn't
            # match the release
            if dirty:
                print(f'major, minor, or patch version must increment',
                      file=sys.stderr)
                exit(1)

            # The pre-release version should not be enabled, we only want one
            # commit where pre-release is disabled per released version
            if PRE_RELEASE:
                print(f"git tag '{tag}' cannot be a pre-release version",
                      file=sys.stderr)
                exit(1)

            print(f'{MAJOR}.{MINOR}.{PATCH}')
            return
        else:
            # The veresion tag does not match what's in this file, and there's
            # no changes. We must've used the wrong tag.
            if not dirty:
                print(f"git tag '{tag}' does not match {MAJOR}.{MINOR}.{PATCH}",
                      file=sys.stderr)
                exit(1)

            # We changed the major version
            if MAJOR != tag_major:
                # We need to only increment the major version by 1
                if MAJOR < tag_major or MAJOR > (tag_major + 1):
                    print(f"major version should be {tag_major + 1}",
                          file=sys.stderr)
                    exit(1)
                # We need to reset the minor and patch versions
                elif MINOR != 0 or PATCH != 0:
                    print("minor and patch version should be 0",
                          file=sys.stderr)
                    exit(1)
            # We changed the minor version
            elif MINOR != tag_minor:
                # We need to only increment the minor version by 1
                if MINOR < tag_minor or MINOR > (tag_minor + 1):
                    print(f"minor version should be {tag_minor + 1}",
                          file=sys.stderr)
                    exit(1)
                # We need to reset the patch version
                elif PATCH != 0:
                    print("patch version should be 0", file=sys.stderr)
                    exit(1)
            # We must have changed the patch version, make sure we only
            # increment the patch version by 1
            elif PATCH != (tag_patch + 1):
                print(f"patch version should be {tag_patch + 1}",
                      file=sys.stderr)
                exit(1)

            print(f'{MAJOR}.{MINOR}.{PATCH}-1+git.{commit}.dirty')
            return

    p = run('git', 'describe', '--abbrev=0', '--match', VERSION_GLOB, 'HEAD')
    if p.returncode == 0:
        most_recent_tag = p.stdout.strip()
        p = run('git', 'rev-list', '--count', f'{most_recent_tag}..HEAD')
    else:
        p = run('git', 'rev-list', '--count', 'HEAD')

    count = int(p.stdout.strip())
    if dirty:
        count += 1
        print(f'{MAJOR}.{MINOR}.{PATCH}-{count}+git.{commit}.dirty')
    else:
        print(f'{MAJOR}.{MINOR}.{PATCH}-{count}+git.{commit}')

if __name__ == '__main__':
    main()
