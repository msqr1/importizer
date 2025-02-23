- This project is a CLI app, it doesn't provide a C++ API for users, so no-one would really need a backward-compatible source interface.
- Therefore, it is best to completely (instead of transitionally) modularize.
- We are runnig it on the `src` folder of the repo on the `1.1.0` tag
- Side note: Yes I am talking about myself, the reason I don't do this now is because linters doesn't support it well, so it is going to look horrible on my editor. I will do this as soon as possible.
```sh
git clone -b 1.1.0 https://github.com/msqr1/importizer
cd importizer
```