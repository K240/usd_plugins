"""Create minimal test USD files for the asset resolver tests.
Run with hython:
    hython create_test_assets.py
"""
import json
import os

from pxr import Usd, UsdGeom

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ASSETS_DIR = os.path.join(SCRIPT_DIR, "test_assets")


def create_chair_usd(path, version_label):
    """Create a minimal USD file with a simple Xform."""
    os.makedirs(os.path.dirname(path), exist_ok=True)
    stage = Usd.Stage.CreateNew(path)
    root = UsdGeom.Xform.Define(stage, "/chair_0")
    root.GetPrim().SetMetadata("comment", f"chair_0 {version_label}")
    import random

    # ランダムにプリミティブを設定する
    prim_types = [
        ('Cube', UsdGeom.Cube),
        ('Sphere', UsdGeom.Sphere),
        ('Cone', UsdGeom.Cone),
        ('Cylinder', UsdGeom.Cylinder),
        ('Capsule', UsdGeom.Capsule),
    ]
    prim_name, prim_class = random.choice(prim_types)
    prim = prim_class.Define(stage, f"/chair_0/{prim_name}")
    prim.GetPrim().SetMetadata("comment", f"Random prim: {prim_name}")

    stage.GetRootLayer().Save()
    print(f"  Created: {path}")


def create_versions_json(asset_dir, name, latest_version):
    """Create versions.json in the asset directory."""
    os.makedirs(asset_dir, exist_ok=True)
    data = {"name": name, "latest": latest_version}
    json_path = os.path.join(asset_dir, "versions.json")
    with open(json_path, "w") as f:
        json.dump(data, f, indent=2)
    print(f"  Created: {json_path}")


if __name__ == "__main__":
    asset_dir = os.path.join(ASSETS_DIR, "chair_0")

    # Create versioned USD files
    for i in range(1, 11):
        create_chair_usd(
            os.path.join(asset_dir, f"v{i}", "chair_0.usd"),
            f"v{i}",
        )

    # Create versions.json (latest -> v10)
    create_versions_json(asset_dir, "chair_0", "v10")

    print("Done.")

