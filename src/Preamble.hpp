#pragma once

struct Opts;
struct File;
struct PreprocessRes;
bool addPreamble(const Opts& opts, File& file, PreprocessRes&& res);