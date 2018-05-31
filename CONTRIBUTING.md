# How to contribute

It is great that you're reading this!
We need volunteer developers to help this project come to fruition.

This guide is intended to direct you toward useful resources with the goal of enabling you to maximize your time and energy to contribute great things to the project.

## Cultivate new ideas

  * [#olimex chat.freenode.net channel](https://webchat.freenode.net/?channels=olimex): Chat about your ideas with like-minded individuals; we want you working on things you're excited about.
  * [TERES-I user forum](https://www.olimex.com/forum/index.php?board=39.0): Ask questions, provide comments and feedback on a wide variety of topics.

## Understand the project

  * [TERES-I Overview](doc/web/dev_overview.md): Where we are
  * [Roadmap](doc/web/dev_roadmap.md): Where we are headed
  * [Wishlist](doc/web/dev_wishlist.md): Where we desire your help the most
  * [Bug and issue reporting](https://github.com/OLIMEX/DIY-LAPTOP/issues): Search for similar outstanding issues before submitting a new one

## Coding process

1. Fork this repository
2. Make local changes using the Git code versioning tool
3. Commit and push the changes to your local fork
4. Submit a [Pull Request (PR) on this main repository](https://github.com/OLIMEX/DIY-LAPTOP/pull/new/master) and select your local branch to be merged

## Coding guidelines

### Coding conventions

Start reading our code and you'll get the hang of it.
We optimize for readability:

  * We indent using four spaces (soft tabs)
  * We ALWAYS put spaces after list items and method parameters (`[1, 2, 3]`, not `[1,2,3]`), around operators (`x += 1`, not `x+=1`), and around hash arrows
  * This is open source software, so please consider the people who will read your code and make it look nice for them
  * Comment your code and provide documentation when appropriate
  * Compose documentation using one sentence/statement per line; this makes modifications easier to follow

### Commit messages

Always write a clear log message for your commits.
Simply saying "Update" is not adequate to understand what change(s) you made.
One-line messages are fine for small changes, but bigger changes should look like this:

```bash
  $ git commit -m "A brief summary of the commit (max 50 chars)
  > 
  > An optional paragraph describing what changed and its impact."
```

Please ensure that all of your commits are atomic (one feature per commit).
This helps prevent tons of headaches digging through code in an attempt to reproduce your steps.

### Branches

Branches should be used for non-trivial changes that would take numerous commits to implement or if such changes could break the build.
Branches are not inherently expected to build; they provide a staging area for your particular fix or feature.

### Testing

Test your code against the master branch **prior** to submitting a PR!
One test per feature is ideal, but more tests should be included for more complicated features.

### Submitting a PR

Please submit a [GitHub Pull Request (PR)](https://github.com/OLIMEX/DIY-LAPTOP/pull/new/master) including a clear list of what you've done (read more about [pull requests](http://help.github.com/pull-requests/)).
Tests and examples provided with your code will help us **tremendously** to understand your contribution and expedite the merge.
We can always use more test coverage.

## Conclusion

We hope that we have not scared you away with formality.
This is a great community that shares like interests and intentions with the project.
If you are reading this, you certainly share much of the same passion as we do.
We look forward to seeing what great ideas you come up with!

Thanks,
TERES Development Team
