"""Test the AssetResolver plugin.
Run with hython after setting up the environment:
    call setup_env.bat
    hython tests/test_resolve.py
"""
import os
import sys
from pxr import Ar, Usd


def test_resolve():
    resolver = Ar.GetResolver()

    asset_root = os.environ.get("USD_ASSET_ROOT", "")
    print(f"USD_ASSET_ROOT = {asset_root}")
    print()

    # --- Test 1: Resolve asset:chair_0 (latest via versions.json -> v10) ---
    path1 = "asset:chair_0"
    resolved1 = resolver.Resolve(path1)
    print(f"[Test 1] Resolve('{path1}')  (latest via versions.json)")
    print(f"  -> {resolved1.GetPathString() if resolved1 else '(empty)'}")
    expected1 = os.path.normpath(
        os.path.join(asset_root, "chair_0", "v10", "chair_0.usd")
    )
    if resolved1:
        assert os.path.normpath(resolved1.GetPathString()) == expected1, (
            f"Expected {expected1}, got {resolved1.GetPathString()}"
        )
        print("  PASS")
    else:
        print(f"  FAIL - expected {expected1}")
    print()

    # --- Test 2: Resolve asset:chair_0?v=10 ---
    path2 = "asset:chair_0?v=10"
    resolved2 = resolver.Resolve(path2)
    print(f"[Test 2] Resolve('{path2}')")
    print(f"  -> {resolved2.GetPathString() if resolved2 else '(empty)'}")
    expected2 = os.path.normpath(
        os.path.join(asset_root, "chair_0", "v10", "chair_0.usd")
    )
    if resolved2:
        assert os.path.normpath(resolved2.GetPathString()) == expected2, (
            f"Expected {expected2}, got {resolved2.GetPathString()}"
        )
        print("  PASS")
    else:
        print(f"  FAIL - expected {expected2}")
    print()

    # --- Test 3: Resolve non-existent version ---
    path3 = "asset:chair_0?v=999"
    resolved3 = resolver.Resolve(path3)
    print(f"[Test 3] Resolve('{path3}')")
    print(f"  -> {resolved3.GetPathString() if resolved3 else '(empty)'}")
    assert not resolved3, "Expected empty resolved path for non-existent version"
    print("  PASS")
    print()

    # --- Test 4: Open stage via asset URI (latest -> v10) ---
    if resolved1:
        print(f"[Test 4] Usd.Stage.Open('{path1}')")
        stage = Usd.Stage.Open(path1)
        if stage:
            prim = stage.GetPrimAtPath("/chair_0")
            comment = prim.GetMetadata("comment") if prim else ""
            print(f"  Root prim: {prim.GetPath() if prim else '(none)'}")
            print(f"  Comment: {comment}")
            assert comment == "chair_0 v10", f"Expected 'chair_0 v10', got '{comment}'"
            print("  PASS")
        else:
            print("  FAIL - could not open stage")
    print()

    # --- Test 5: CreateIdentifier ---
    identifier = resolver.CreateIdentifier(path2)
    print(f"[Test 5] CreateIdentifier('{path2}')")
    print(f"  -> {identifier}")
    assert identifier == path2, f"Expected '{path2}', got '{identifier}'"
    print("  PASS")
    print()

    print("All tests completed.")


if __name__ == "__main__":
    test_resolve()
