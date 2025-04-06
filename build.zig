// SPDX-License-Identifier: MIT License
const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    var CXX_FLAGS = std.ArrayList([]const u8).init(b.allocator);
    CXX_FLAGS.appendSlice(&.{
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
    }) catch @panic("OOM");

    if (optimize == .Debug) {
        CXX_FLAGS.appendSlice(&.{
            "-DDEBUG=1", //
            "-ggdb3", //
            "-pg", //
            "-O0", //
        }) catch @panic("OOM");
    }

    const exe = b.addExecutable(.{
        .name = "bf-cc",
        .target = target,
        .optimize = optimize,
    });
    exe.addIncludePath(b.path("src"));
    exe.linkLibC();
    exe.linkLibCpp();
    exe.addCSourceFiles(.{
        .root = b.path("src"),
        .files = &.{
            "bf.cc",
            "platform_linux.cc",
            "platform_windows.cc",
            "assembler_x86_64.cc",
            "assembler_aarch64.cc",
            "compiler.cc",
            "debug.cc",
            "error.cc",
            "instr.cc",
            "interp.cc",
            "mem.cc",
            "optimize.cc",
            "opt_comment_loop.cc",
            "opt_delay_ptr.cc",
            "opt_double_guard.cc",
            "opt_fusion_op.cc",
            "opt_multiply_loop.cc",
            "opt_peep.cc",
            "parse.cc",
        },
        .flags = CXX_FLAGS.items
    });

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run bf-cc");
    run_step.dependOn(&run_cmd.step);

    const googletest_dep = b.dependency("googletest", .{
        .target = target,
        .optimize = optimize,
    });

    const test_exe = b.addExecutable(.{
        .name = "bf-cc-test",
        .target = target,
        .optimize = optimize,
    });
    test_exe.addIncludePath(b.path("src"));
    test_exe.addIncludePath(b.path("test"));
    test_exe.linkLibC();
    test_exe.linkLibCpp();
    test_exe.linkLibrary(googletest_dep.artifact("gtest"));
    test_exe.addCSourceFiles(.{
        .root = b.path("test"),
        .files = &.{
            "main.cc",
            "test_interp.cc",
            "test_opt_comment_loop.cc",
            "test_opt_double_guard.cc",
            "test_opt_fusion_op.cc",
            "test_opt_multiply_loop.cc",
        },
        .flags = CXX_FLAGS.items,
    });
    test_exe.addCSourceFiles(.{
        .root = b.path("src"),
        .files = &.{
            "platform_linux.cc",
            "platform_windows.cc",
            "assembler_x86_64.cc",
            "assembler_aarch64.cc",
            "compiler.cc",
            "debug.cc",
            "error.cc",
            "instr.cc",
            "interp.cc",
            "mem.cc",
            "optimize.cc",
            "opt_comment_loop.cc",
            "opt_delay_ptr.cc",
            "opt_double_guard.cc",
            "opt_fusion_op.cc",
            "opt_multiply_loop.cc",
            "opt_peep.cc",
            "parse.cc",
        },
        .flags = CXX_FLAGS.items,
    });

    const install_test_exe = b.addInstallArtifact(test_exe, .{});

    const test_step = b.step("test", "Build the test executable");
    test_step.dependOn(&install_test_exe.step);

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

    const check_step = b.step("check", "Run the unit and integration tests");
    check_step.dependOn(&run_unit_tests.step);
    check_step.dependOn(&run_integration_tests.step);
}
