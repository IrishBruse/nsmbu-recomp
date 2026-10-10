#!/usr/bin/env python3
"""Unit tests for the build registry: python3 test_builds.py

No game files: the registry holds title ids and SHA-256 digests only.
"""
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import builds

HERE = os.path.dirname(os.path.abspath(__file__))

class Registry(unittest.TestCase):
    def test_canonical_is_usa_and_identity(self):
        usa = builds.canonical_build()
        self.assertEqual(usa.name, "USA")
        self.assertEqual(usa.title_id, "0005000010101d00")
        self.assertEqual(
            usa.sha256,
            "ebd147ce4cdedb563db3ac6278186ca5dea282ad315271925b8bb5ac16c7dc8e",
        )
        self.assertTrue(usa.canonical)
        for a in (0x02000020, 0x025F172C, 0x028F8250, 0x101F4BAC, 0x1046F0B0):
            self.assertEqual(usa.code(a), a)
            self.assertEqual(usa.data(a), a)
            self.assertFalse(usa.body_differs(a))

    def test_known_builds(self):
        names = {b.name for b in builds.all_builds()}
        self.assertEqual(names, {"USA"})
        self.assertEqual(builds.by_title("0005000010101d00").name, "USA")
        self.assertIsNone(builds.by_title("0005000010143500"))
        self.assertIsNone(builds.by_name("eu"))

    def test_titles_and_hashes_are_distinct(self):
        all_builds = builds.all_builds()
        self.assertEqual(len({b.title_id for b in all_builds}), len(all_builds))
        self.assertEqual(len({b.sha256 for b in all_builds}), len(all_builds))
        for b in all_builds:
            self.assertRegex(b.sha256, r"^[0-9a-f]{64}$")
            self.assertRegex(b.title_id, r"^00050000[0-9a-f]{8}$")

class Identify(unittest.TestCase):
    def test_a_file_that_is_no_build(self):
        with open(__file__, "rb"):
            self.assertIsNone(builds.identify(__file__))

    def test_by_sha256(self):
        usa = builds.canonical_build()
        self.assertEqual(builds.by_sha256(usa.sha256).name, "USA")
        self.assertIsNone(builds.by_sha256("0" * 64))

class Hooks(unittest.TestCase):
    def test_every_file_is_read_for_the_canonical_build(self):
        entries, skipped = builds.read_hooks(builds.hook_files(), builds.canonical_build())
        self.assertEqual(skipped, [])
        self.assertEqual(
            [addr for _, _, addr, _ in entries],
            [0x022A79C4],
        )
        for site, canon, addr, where in entries:
            self.assertEqual(canon, addr, where)
            self.assertTrue(site, where)

    def test_build_directive_skips_a_file(self):
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, "hooks_language.txt")
            with open(path, "w", encoding="utf-8") as source:
                source.write("# builds: USA\n022A79C4\n")
            other = builds.Build({
                "name": "EU",
                "title_id": "0005000010101e00",
                "rpx_sha256": "0" * 64,
                "code_bounds": ["02000020", "028F87F4"],
                "data_bounds": ["10000000", "104DA1C8"],
            })
            entries, skipped = builds.read_hooks([path], other)
        self.assertEqual(skipped, [("hooks_language.txt", "USA")])
        self.assertEqual(entries, [])

if __name__ == "__main__":
    unittest.main(verbosity=2)
