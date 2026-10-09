#!/usr/bin/env python3
import argparse
import glob
import os
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

BUILD = os.path.join(ROOT, "build")


def debug_compile_flags(install_flags, sanitizer=False):
    flags = install_flags
    if sanitizer:
        flags = f"{flags} -fsanitize=address,undefined -fno-omit-frame-pointer"
    return flags


def debug_link_flags(link_dir, sanitizer=False):
    flags = f"-L{link_dir}"
    if sanitizer:
        flags = f"{flags} -fsanitize=address,undefined"
    return flags


def link_compile_commands(build_dir):
    src = os.path.join(build_dir, "compile_commands.json")
    dst = os.path.join(ROOT, "compile_commands.json")
    if os.path.isfile(src):
        if os.path.islink(dst) or os.path.isfile(dst):
            os.remove(dst)
        os.symlink(os.path.relpath(src, ROOT), dst)


def extra_cmake_debug(mods=False):
    if mods:
        return ["-DNSMBU_MODS_ENABLED=ON"]
    return []


def libstdcxx_libdir():
    for ver in range(20, 10, -1):
        hits = glob.glob(f"/usr/lib/gcc/*-linux-gnu/{ver}/libstdc++.so")
        if hits:
            return os.path.dirname(hits[0])
    return None


def gcc_install_flags(libdir):
    return [f"--gcc-install-dir={libdir}"]


def clang_pair(libdir):
    cxx_flags = gcc_install_flags(libdir)
    for base in ("clang-19", "clang-18", "clang-17", "clang"):
        cc = shutil.which(base)
        if not cc:
            continue
        if base == "clang":
            cxx = shutil.which("clang++")
        else:
            cxx = shutil.which(base.replace("clang", "clang++", 1))
        if not cxx:
            continue
        with tempfile.TemporaryDirectory(dir=os.path.join(ROOT, ".tmp")) as tmp:
            src = os.path.join(tmp, "t.cpp")
            exe = os.path.join(tmp, "t")
            with open(src, "w", encoding="utf-8") as handle:
                handle.write("int main() { return 0; }\n")
            link = ["-L" + libdir, "-lstdc++"] if libdir else []
            r = subprocess.run(
                [cxx, *cxx_flags, src, "-o", exe, *link],
                capture_output=True,
                text=True,
            )
            if r.returncode == 0:
                return cc, cxx
    return None, None


def gen_is_stub(gen):
    report = os.path.join(gen, "report.txt")
    if not os.path.isfile(report):
        return True
    with open(report, encoding="utf-8", errors="replace") as handle:
        return handle.readline().startswith("placeholder from stubgen.py")


def ensure_gen_dir():
    gen = os.path.join(ROOT, "build", "gen")
    rpx = os.path.join(ROOT, "game", "code", "red-pro2.rpx")
    os.makedirs(gen, exist_ok=True)
    if os.path.isfile(rpx):
        table = os.path.join(gen, "table.c")
        stale = (
            gen_is_stub(gen)
            or not os.path.isfile(table)
            or os.path.getmtime(rpx) > os.path.getmtime(table)
        )
        if stale:
            subprocess.run(
                [sys.executable, os.path.join(ROOT, "tools", "recomp", "recomp.py"), rpx, gen],
                check=True,
                cwd=ROOT,
            )
        return
    if glob.glob(os.path.join(gen, "code_*.c")) and not gen_is_stub(gen):
        return
    subprocess.run(
        [sys.executable, os.path.join(ROOT, "tools", "recomp", "stubgen.py"), gen],
        check=True,
        cwd=ROOT,
    )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--release", action="store_true")
    parser.add_argument("--no-debug", action="store_true")
    parser.add_argument("--sanitizer", action="store_true")
    parser.add_argument("--mods", action="store_true")
    parser.add_argument("cmake_args", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    debug_build = not args.release and not args.no_debug
    ensure_gen_dir()
    libdir = libstdcxx_libdir()
    if not libdir:
        sys.exit(
            "build needs libstdc++ for Clang (install libstdc++-14-dev or another gcc libstdc++ dev package)"
        )
    cc, cxx = clang_pair(libdir)
    if not cc:
        sys.exit("build needs a working clang++ on PATH (clang-18 or clang)")
    install = " ".join(gcc_install_flags(libdir))
    compile_flags = debug_compile_flags(install, args.sanitizer)
    link_flags = debug_link_flags(libdir, args.sanitizer)
    cmake = [
        "cmake",
        "-S",
        ROOT,
        "-B",
        BUILD,
        f"-DCMAKE_C_COMPILER={cc}",
        f"-DCMAKE_CXX_COMPILER={cxx}",
        f"-DCMAKE_C_FLAGS={compile_flags}",
        f"-DCMAKE_CXX_FLAGS={compile_flags}",
        f"-DCMAKE_EXE_LINKER_FLAGS={link_flags}",
        f"-DCMAKE_SHARED_LINKER_FLAGS={link_flags}",
        f"-DCMAKE_MODULE_LINKER_FLAGS={link_flags}",
    ]
    if args.release:
        cmake.append("-DCMAKE_BUILD_TYPE=Release")
    elif debug_build:
        cmake.append("-DCMAKE_BUILD_TYPE=Debug")
    else:
        cmake.append("-DCMAKE_BUILD_TYPE=RelWithDebInfo")
    extra = list(args.cmake_args)
    if not any(a.startswith("-DNSMBU_BUNDLED_DEPS") for a in extra):
        cmake.append("-DNSMBU_BUNDLED_DEPS=ON")
    cmake.extend(extra_cmake_debug(args.mods))
    cmake.extend(extra)
    subprocess.run(cmake, check=True, cwd=ROOT)
    subprocess.run(
        ["cmake", "--build", BUILD, "--target", "nsmbu", "-j", str(args.jobs)],
        check=True,
        cwd=ROOT,
    )
    if debug_build:
        link_compile_commands(BUILD)


if __name__ == "__main__":
    main()
