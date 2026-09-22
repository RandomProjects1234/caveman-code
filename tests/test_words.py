import unittest

from tests.context import SRC  # noqa: F401

from cmc import words


class TestWords(unittest.TestCase):
    def test_every_word_has_docs(self):
        for word in words.all_words():
            self.assertTrue(word.get("summary"), word["name"])
            self.assertTrue(word.get("doc"), word["name"])
            self.assertTrue(word.get("syntax"), word["name"])
            self.assertTrue(word.get("example"), word["name"])
            self.assertIn(word.get("category"), words.CATEGORIES, word["name"])

    def test_names_unique(self):
        names = [w["name"] for w in words.all_words()]
        self.assertEqual(len(names), len(set(names)))

    def test_aliases_resolve(self):
        self.assertEqual(words.find_alias("print"), "oga")
        self.assertEqual(words.find_alias("if"), "binga")
        self.assertEqual(words.find_alias("true"), "gronk")
        self.assertEqual(words.find_alias("nothing"), "plop")
        self.assertEqual(words.find_alias("def"), "clump")
        self.assertEqual(words.find_alias("end"), "unga")

    def test_builtin_arities_are_sane(self):
        for word in words.BUILTIN_WORDS:
            low, high = word["arity"]
            self.assertLessEqual(low, high, word["name"])
            self.assertGreaterEqual(high, 0, word["name"])

    def test_word_info_lookup(self):
        info = words.word_info("oga")
        self.assertIsNotNone(info)
        self.assertEqual(info["category"], "Talking")
        self.assertIsNone(words.word_info("not-a-word"))


if __name__ == "__main__":
    unittest.main()