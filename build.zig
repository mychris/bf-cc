const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const CXX_FLAGS = &.{
        "--std=c++20", //
        "-pedantic", //
        "-Wall", //
        "-Wextra", //
        "-Wall", //
        "-Wextra", //
        "-Wformat", //
        "-Wreturn-type", //
        "-Wstrict-aliasing", //
        "-Wcast-qual", //
        "-Wcast-align", //
        "-Wconversion", //
        "-Wwrite-strings", //
        "-Wsign-conversion", //
        "-Wshadow", //
        "-pedantic", //
        "-fno-exceptions", //
        "-fno-rtti", //
        "-fPIE", //
        "-fstack-clash-protection", //
        "-fstack-protector-strong",
    };

    const exe = b.addExecutable(.{
        .name = "bf-cc",
        .target = target,
        .optimize = optimize,
    });
    exe.addIncludePath(.{ .path = "src" });
    exe.linkLibC();
    exe.linkLibCpp();
    exe.addCSourceFiles(&.{
        "src/bf.cc",
        "src/assembler.cc",
        "src/error.cc",
        "src/instr.cc",
        "src/interp.cc",
        "src/mem.cc",
        "src/optimize.cc",
        "src/opt_comment_loop.cc",
        "src/opt_delay_ptr.cc",
        "src/opt_double_guard.cc",
        "src/opt_fusion_op.cc",
        "src/opt_multiply_loop.cc",
        "src/opt_peep.cc",
        "src/parse.cc",
    }, CXX_FLAGS);

    b.installArtifact(exe);

    const gtest_root = "/usr/src/googletest/";

    const lib_gtest = b.addStaticLibrary(.{
        .name = "gtest",
        .target = target,
        .optimize = optimize,
    });
    lib_gtest.linkLibC();
    lib_gtest.linkLibCpp();
    lib_gtest.addIncludePath(.{ .path = gtest_root });
    lib_gtest.addCSourceFiles(&.{
        gtest_root ++ "src/gtest-all.cc",
    }, CXX_FLAGS);

    const test_exe = b.addExecutable(.{
        .name = "bf-cc-test",
        .target = target,
        .optimize = optimize,
    });
    test_exe.addIncludePath(.{ .path = "src" });
    test_exe.linkLibC();
    test_exe.linkLibCpp();
    test_exe.linkLibrary(lib_gtest);
    test_exe.addCSourceFiles(&.{
        "test/main.cc",
        "test/test_interp.cc",
        "test/test_opt_comment_loop.cc",
        "test/test_opt_double_guard.cc",
        "test/test_opt_fusion_op.cc",
        "test/test_opt_multiply_loop.cc",
        "src/assembler.cc",
        "src/error.cc",
        "src/instr.cc",
        "src/interp.cc",
        "src/mem.cc",
        "src/optimize.cc",
        "src/opt_comment_loop.cc",
        "src/opt_delay_ptr.cc",
        "src/opt_double_guard.cc",
        "src/opt_fusion_op.cc",
        "src/opt_multiply_loop.cc",
        "src/opt_peep.cc",
        "src/parse.cc",
    }, CXX_FLAGS);

    const run_unit_tests = b.addRunArtifact(test_exe);
    run_unit_tests.addArgs(&.{
        "--gtest_shuffle",
        "--gtest_color=no",
        "--gtest_brief=1",
    });
    const run_integration_tests = b.addSystemCommand(&.{
        "env",
        "VERBOSE=0",
        "./t/test.bash",
    });
    run_integration_tests.addArtifactArg(exe);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_unit_tests.step);
    test_step.dependOn(&run_integration_tests.step);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run bf-cc");
    run_step.dependOn(&run_cmd.step);
}
