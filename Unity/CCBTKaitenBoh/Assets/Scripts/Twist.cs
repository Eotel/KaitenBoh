using UnityEngine;

public class Twist : MonoBehaviour
{
    /// <summary>
    ///     <paramref name="ra" />から<paramref name="rb" />への回転から、軸<paramref name="axis" />に関するねじれ角を得ます。
    /// </summary>
    /// <param name="ra">起点の回転。</param>
    /// <param name="rb">終点の回転。</param>
    /// <param name="axis">ねじれ軸。</param>
    /// <returns>軸<paramref name="axis" />に関する0°以上360°未満のねじれ角。</returns>
    public static float GetTwistAroundAxis(Quaternion ra, Quaternion rb, Vector3 axis)
    {
        // 軸を正規化する
        if (axis == Vector3.zero)
        {
            axis = Vector3.forward;
        }

        axis.Normalize();

        // da、db、rab、rdadbを求める
        var da = ra * axis;
        var db = rb * axis;
        var rab = rb * Quaternion.Inverse(ra);
        var rdadb = Quaternion.FromToRotation(da, db);

        // rdadbからrabへの回転を求めたのち、その軸と角度を抽出する
        var delta = rab * Quaternion.Inverse(rdadb);
        delta.ToAngleAxis(out var deltaAngle, out var deltaAxis);

        // dbとdeltaAxisは同一直線上にあるはずだが、向きは逆かもしれない
        // 角度の正負を統一するため、向きの逆転の有無を調べる
        // deltaAngleSignはdbとdeltaAxisの向きが一致していれば1、逆転していれば-1になる
        var deltaAngleSign = Mathf.Sign(Vector3.Dot(db, deltaAxis));

        // 角度の符号を補正した上で0°～360°におさめて返す
        var result = (deltaAngleSign * deltaAngle) % 360.0f;
        if (result < 0.0f)
            result += 360.0f;

        return result;
    }
}