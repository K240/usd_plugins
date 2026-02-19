"""Test the AssetInfoAPI codeless schema.
Run with hython after setting up the environment:
    call setup_env.bat
    hython tests/test_schema.py
"""
import os
from pxr import Usd, Sdf


def test_schema():
    print("=== AssetInfoAPI Codeless Schema Tests ===")
    print()

    # --- Test 1: Apply schema to a prim ---
    print("[Test 1] Apply AssetInfoAPI to a prim")
    stage = Usd.Stage.CreateInMemory()
    prim = stage.DefinePrim("/TestAsset", "Xform")

    result = prim.ApplyAPI("AssetInfoAPI")
    assert result, "Failed to apply AssetInfoAPI"
    assert prim.HasAPI("AssetInfoAPI"), "Prim should have AssetInfoAPI applied"
    print("  Applied: OK")
    print("  HasAPI:  OK")
    print("  PASS")
    print()

    # --- Test 2: Set and get attributes ---
    print("[Test 2] Set and get schema attributes")
    prim.GetAttribute("assetInfo:version").Set("v10")
    prim.GetAttribute("assetInfo:author").Set("hideki")
    prim.GetAttribute("assetInfo:status").Set("published")
    prim.GetAttribute("assetInfo:description").Set("A test chair asset")

    assert prim.GetAttribute("assetInfo:version").Get() == "v10"
    assert prim.GetAttribute("assetInfo:author").Get() == "hideki"
    assert prim.GetAttribute("assetInfo:status").Get() == "published"
    assert prim.GetAttribute("assetInfo:description").Get() == "A test chair asset"
    print("  version:     v10")
    print("  author:      hideki")
    print("  status:      published")
    print("  description: A test chair asset")
    print("  PASS")
    print()

    # --- Test 3: Default values ---
    print("[Test 3] Check default values")
    stage2 = Usd.Stage.CreateInMemory()
    prim2 = stage2.DefinePrim("/Defaults", "Xform")
    prim2.ApplyAPI("AssetInfoAPI")

    assert prim2.GetAttribute("assetInfo:version").Get() == ""
    assert prim2.GetAttribute("assetInfo:author").Get() == ""
    assert prim2.GetAttribute("assetInfo:status").Get() == "draft"
    assert prim2.GetAttribute("assetInfo:description").Get() == ""
    print("  version:     '' (empty)")
    print("  author:      '' (empty)")
    print("  status:      draft")
    print("  description: '' (empty)")
    print("  PASS")
    print()

    # --- Test 4: Schema survives export/import ---
    print("[Test 4] Export and re-import with schema")
    tmp_path = os.path.join(os.path.dirname(__file__), "test_assets", "_tmp_schema_test.usda")
    os.makedirs(os.path.dirname(tmp_path), exist_ok=True)
    stage.GetRootLayer().Export(tmp_path)

    stage3 = Usd.Stage.Open(tmp_path)
    prim3 = stage3.GetPrimAtPath("/TestAsset")
    assert prim3.HasAPI("AssetInfoAPI"), "Re-imported prim should have AssetInfoAPI"
    assert prim3.GetAttribute("assetInfo:version").Get() == "v10"
    assert prim3.GetAttribute("assetInfo:status").Get() == "published"
    print(f"  Exported to: {tmp_path}")
    print("  Re-imported: OK")
    print("  Attributes preserved: OK")
    print("  PASS")

    os.remove(tmp_path)
    print()

    # --- Test 5: Use with asset resolver ---
    asset_root = os.environ.get("USD_ASSET_ROOT", "")
    if asset_root:
        print("[Test 5] Apply schema to asset-resolved stage")
        stage4 = Usd.Stage.Open("asset:chair_0")
        if stage4:
            prim4 = stage4.GetPrimAtPath("/chair_0")
            prim4.ApplyAPI("AssetInfoAPI")
            prim4.GetAttribute("assetInfo:version").Set("v10")
            prim4.GetAttribute("assetInfo:status").Set("published")
            assert prim4.HasAPI("AssetInfoAPI")
            assert prim4.GetAttribute("assetInfo:version").Get() == "v10"
            print("  Stage opened via asset:chair_0")
            print("  AssetInfoAPI applied and attributes set")
            print("  PASS")
        else:
            print("  SKIP - could not open asset:chair_0")
    else:
        print("[Test 5] SKIP - USD_ASSET_ROOT not set")
    print()

    print("All tests completed.")


if __name__ == "__main__":
    test_schema()
