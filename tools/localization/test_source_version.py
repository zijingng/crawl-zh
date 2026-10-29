"""Fork versions must not depend on which old tags a server happens to have."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


class SourceVersionTest(unittest.TestCase):
    def setUp(self):
        if not shutil.which("perl") or not shutil.which("git"):
            self.skipTest("Perl and Git required")
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.repo = self.root/"repo"
        self.repo.mkdir()
        self.git("init", "-q")
        self.git("config", "user.name", "Version test")
        self.git("config", "user.email", "version@example.invalid")
        self.git("commit", "-q", "--allow-empty", "-m", "upstream baseline")
        self.base = self.git("rev-parse", "HEAD")
        self.git("tag", "-a", "0.25-a0", "-m", "old tag")
        self.git("commit", "-q", "--allow-empty", "-m", "translation")
        self.install(self.repo)

    def git(self, *args):
        return subprocess.check_output(["git", *args], cwd=self.repo, text=True).strip()

    def install(self, folder):
        util = folder/"util"
        util.mkdir(exist_ok=True)
        source = Path(__file__).resolve().parents[2]/"crawl-ref/source/util"
        for name in ["source-version.pl", "gen_ver.pl"]:
            shutil.copyfile(source/name, util/name)
        (util/"version_base").write_text("0.34.1 " + self.base + "\n", encoding="utf-8")

    def version(self, folder, *args):
        return subprocess.check_output(["perl", "util/source-version.pl", *args],
                                       cwd=folder, text=True).strip()

    def test_old_tags_do_not_override_confirmed_upstream_baseline(self):
        self.assertTrue(self.git("describe").startswith("0.25-a0-"))
        self.assertEqual(self.version(self.repo), "0.34.1-1-g" + self.git("rev-parse", "--short=10", "HEAD"))
        self.assertEqual(self.version(self.repo, self.base), "0.34.1")
        subprocess.run(["perl", "util/gen_ver.pl", "build.h"], cwd=self.repo, check=True)
        header = (self.repo/"build.h").read_text()
        self.assertIn('CRAWL_VERSION_MAJOR "0.34"', header)
        self.assertIn('CRAWL_VERSION_SHORT "0.34.1"', header)
        self.assertNotIn("0.25", header)

    def test_shallow_clone_keeps_release_and_actual_revision(self):
        # Use Git's actual shallow boundary without relying on platform-specific
        # file:// URL conventions or a local transport implementation.
        head = self.git("rev-parse", "HEAD")
        (self.repo/".git/shallow").write_text(head + "\n")
        self.assertEqual(self.git("rev-parse", "--is-shallow-repository"), "true")
        sha = self.git("rev-parse", "--short=10", "HEAD")
        self.assertEqual(self.version(self.repo), "0.34.1-shallow-g" + sha)

    def test_archive_preserves_packaged_revision_or_known_release(self):
        archive = self.root/"archive"
        archive.mkdir()
        self.install(archive)
        self.assertEqual(self.version(archive), "0.34.1")
        (archive/"util/release_ver").write_text("0.34.1-3-gabcdef0123\n")
        self.assertEqual(self.version(archive), "0.34.1-3-gabcdef0123")
