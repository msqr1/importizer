#pragma once

struct Opts;
struct File;
struct PreprocessRes;
bool addPreamble(File& file, PreprocessRes&& res, const Opts& opts);