#pragma once

struct Opts;
struct File;
struct PreprocessRes;
void addPreamble(const Opts& opts, File& file, PreprocessRes&& res, bool exported);