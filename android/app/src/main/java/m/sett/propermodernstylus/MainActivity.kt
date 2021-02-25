package m.sett.propermodernstylus

import android.annotation.SuppressLint
import android.os.Bundle
import android.util.Log
import android.view.MotionEvent
import android.view.MotionEvent.ACTION_DOWN
import android.view.MotionEvent.ACTION_HOVER_ENTER
import android.view.MotionEvent.ACTION_HOVER_EXIT
import android.view.MotionEvent.ACTION_HOVER_MOVE
import android.view.MotionEvent.ACTION_MOVE
import android.view.MotionEvent.ACTION_UP
import android.view.MotionEvent.TOOL_TYPE_FINGER
import android.view.MotionEvent.TOOL_TYPE_STYLUS
import android.view.WindowManager
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*
import kotlin.math.roundToInt

class MainActivity : AppCompatActivity() {
    private val TAG = "PMS"

    val targetSize = Pair(1920.toDouble(), 1080.toDouble())
    var lastPressure = 0

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        window.setFlags(
            WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN
        )

        var viewSize = Pair(root.width.toDouble(), root.height.toDouble())
        root.post {
            viewSize = Pair(root.width.toDouble(), root.height.toDouble())
        }

        root.setOnHoverListener { v, e ->
            // Log.d("HOLA", "$e")
            if (e.getToolType(0) == TOOL_TYPE_STYLUS) {
                when (e.action) {
                    ACTION_HOVER_ENTER -> {
                        send("hd") // hover down
                    }
                    ACTION_HOVER_EXIT -> {
                        send("hu") // hover up
                    }
                    ACTION_HOVER_MOVE -> {
                        val x = e.x.toDouble() / viewSize.first * targetSize.first
                        val y = e.y.toDouble() / viewSize.second * targetSize.second
                        if (x < 9999 && 0 < x && y < 9999 && 0 < y) {
                            send("n${x.roundToInt()},${y.roundToInt()}:${if (e.buttonState == MotionEvent.BUTTON_STYLUS_PRIMARY) 1 else 0}") // hover move x,y:buttonPressed
                        }
                    }
                }
            }
            false
        }

        root.setOnTouchListener { v, e ->
            when (e.getToolType(0)) {
                TOOL_TYPE_STYLUS -> {
                    when (e.action) {
                        ACTION_DOWN -> {
                            send("sd") // screen/stylus down
                        }
                        ACTION_UP -> {
                            send("su") // screen/stylus up
                        }
                        ACTION_MOVE -> {
                            if (e.pressure * 1024 > 100) {
                                lastPressure = (e.pressure * 1024).roundToInt()
                            } else {
                                lastPressure /= 2
                            }
                            val x = e.x.toDouble() / viewSize.first * targetSize.first
                            val y = e.y.toDouble() / viewSize.second * targetSize.second
                            if (x < 9999 && 0 < x && y < 9999 && 0 < y) {
                                send("m${x.roundToInt()},${y.roundToInt()}:${if (e.buttonState == MotionEvent.BUTTON_STYLUS_PRIMARY) 1 else 0}:$lastPressure") // hover move x,y:buttonPressed
                            }
                        }
                    }
                }
                TOOL_TYPE_FINGER -> {
                    when (e.actionIndex) {
                        // 1 -> send("2") // two finger tap
                        2 -> send("3") // three finger tap / change screen
                    }
                }
            }
            true
        }
    }

    private fun send(msg: String) {
        output.text = msg
        Log.d(TAG, msg)
    }
}
